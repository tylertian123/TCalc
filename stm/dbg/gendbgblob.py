#!/usr/bin/env python3
"""
usage:

    gendbgblob.py <elf file> <blob.bin>

outputs:

    - symtable:
        
        massive table of <start address> <name, null terminateD>

        size is inferred from linker variables

        usually stored gzip-compressed, and searched through with a simple loop

    - frametable:

        another massive table of <start address> <end address> <unwind quantity>

        if current address is between start and end (inclusive, exclusive), one can unwind the frame by popping that many bytes.
        after doing this pop, the top of the stack is the next PC, and the process repeats. If the unwind quantity (stored as 16 bit signed) is -32768,
        assume the function is a leaf function and the next PC is in LR. If the unwind is negative but _not_ -32768, still assume the PC is in LR, but pop that 
        much data.
    
    these are stored in blob.bin as two separately compressed gzip blobs. the only header is the 4-byte little endian size of the symtable

"""
from elftools.elf.elffile import ELFFile
from elftools.dwarf.callframe import FDE, CIE, instruction_name
from elftools.dwarf import constants as dwc
import sys
import struct
import cxxfilt
import zlib
import io

# max length for symbols
SYMBOL_CUTOFF = 64

def lowmemcompress(data):
    cobj = zlib.compressobj(level=9, wbits=11)
    return cobj.compress(data) + cobj.flush()

def run(inelf, outbin):
    elf = ELFFile(open(inelf, "rb"))

    # go trhough all symbols

    symtable = {}
    addr_set = []
    symtable_section = elf.get_section_by_name(".symtab")

    for symbol in symtable_section.iter_symbols():
        if not symbol.name.startswith("_Z"):
            continue
        if elf.get_section(symbol.entry.st_shndx).name != ".text":
            continue
        symtable[symbol.name] = symbol.entry.st_value
        addr_set.append(symbol.entry.st_value)

    # generate debug table

    dbgtable = {}
    dbgdata = elf.get_dwarf_info()

    # go over dbg CFI (common informations)
    fdes = [x for x in dbgdata.CFI_entries() if isinstance(x, FDE)]
    fde: FDE

    for fde in fdes:
        cie: CIE = fde.cie

        base_address = fde.header.initial_location
        if base_address < 0x08000000:  # not in flash
            continue
            
        instructions = cie.instructions + fde.instructions

        block_start = base_address
        block_end   = block_start
        cfe_offset = 0
        pc_addr_offset = 0
        is_lx      = True

        LX_REGNO = 14

        code_mult = cie.header.code_alignment_factor
        data_mult = cie.header.data_alignment_factor

        stack = []

        def do_add():
            nonlocal block_start, block_end

            if is_lx and cfe_offset == 0:
                unwind_offset = -32768
            elif is_lx:
                unwind_offset = -cfe_offset
            else:
                unwind_offset = cfe_offset + pc_addr_offset

            dbgtable[(block_start, block_end)] = unwind_offset
            block_start = block_end

        for instruction in instructions:
            opcode = instruction_name(instruction.opcode)
            if opcode.startswith("DW_CFA_advance_loc") and opcode[-1] in "c1248":
                block_end += instruction.args[0] * code_mult
                if block_start != block_end:
                    do_add()
            elif opcode == "DW_CFA_set_loc":
                block_end = instruction.args[0]
                if block_start != block_end:
                    do_add()
            elif opcode.startswith("DW_CFA_def_cfa_offset"):
                cfe_offset = instruction.args[0]
            elif opcode.startswith("DW_CFA_offset"):
                if instruction.args[0] == LX_REGNO:
                    # are we changing the location of r14?
                    is_lx = False
                    pc_addr_offset = instruction.args[1] * data_mult
            elif opcode == "DW_CFA_remember_state":
                stack.append((cfe_offset, pc_addr_offset, is_lx))
            elif opcode == "DW_CFA_restore_state":
                (cfe_offset, pc_addr_offset, is_lx) = stack.pop(-1)
            elif opcode == "DW_CFA_restore" or opcode == "DW_CFA_restore_extended":
                if instruction.args[0] == LX_REGNO:
                    is_lx = True
                    pc_addr_offset = 0

        block_end = base_address + fde.header.address_range 
        do_add()

    # output in sorted form

    f1 = io.BytesIO()  # symbol table
    f2 = io.BytesIO()  # unwind table

    for name, addr in sorted(symtable.items(), key=lambda x: x[0]):
        f1.write(struct.pack(">I", addr))
        f1.write(cxxfilt.demangleb(name.encode('ascii'))[:SYMBOL_CUTOFF])
        f1.write(b'\x00')

    for (start, end), offs in sorted(dbgtable.items(), key=lambda x: x[0][0]):
        f2.write(struct.pack(">IIh", start, end, offs))

    blob1 = lowmemcompress(f1.getvalue())
    blob2 = lowmemcompress(f2.getvalue())

    with open(outbin, "wb") as f:
        f.write(struct.pack("<I", len(blob1)))
        f.write(blob1)
        f.write(blob2)

if __name__ == "__main__":
    run(*sys.argv[1:])
