#include <gunzip.h>
#include "exception.hpp"
#include <unwind.h>
#include "usart.hpp"
#include <string.h>

#ifndef DEBUG_EXCEPTION
#define DEBUG_EXCEPTION 0
#endif

#define DEBUG_EXCEPTION_PRINT(fmt, ...) \
	do { if (DEBUG_EXCEPTION) printf(fmt, __VA_ARGS__); } while (0)

// Definitions to match with external data

extern "C" {
	// Defined in the linker
	extern const uint32_t dbg_symtable_size;
	extern const uint8_t dbg_data_begin;

	// Derived in nicer types
	extern const uint8_t * const dbg_symtable_begin = &dbg_data_begin;
	extern const uint8_t * const dbg_backtable_begin = (&dbg_data_begin) + dbg_symtable_size;

	// Top of stack
	extern const uint32_t _estack;
};

namespace exception {

	void _applyUnwindTo(uint32_t &PC, uint32_t &LR, uint32_t &SP, int16_t unwind_amt) {
		// If unwind_amt is positive, then we add that much to SP, deref to get PC, and add 4 to SP.
		if (unwind_amt >= 0) {
			SP += unwind_amt;
			PC = *reinterpret_cast<uint32_t *>(SP);
			SP += 4;
		}
		// If unwind_amt is negative max, there was no stack allocated.
		else if (unwind_amt == -32768) {
			PC = LR;
		}
		else {
			// Otherwise, add -unwind_amt to SP, and use LR
			SP += -unwind_amt;
			PC = LR;
		}
	}

	// Sliding window backtrace parser
	
	struct BackTraceParseState {
		uint8_t parse_state = 0;  // Upper 4 bits = state, lower 4 bits = index
								  // where state = (
								  // 	0 - nothing
								  // 	1 - getting addr 1
								  // 	2 - getting addr 2
								  // 	3 - getting unwind amt
								  // )
								  // where index is the byte into the value
		uint32_t lower_PC = 0, upper_PC = 0;
		int16_t  unwind_amt = 0;
		
		uint32_t PC;
		uint32_t LR;
		uint32_t SP;

		uint32_t * backtrace;
		uint16_t   bt_len;
		uint16_t   bt_maxlen;

		bool signalDone;
		ptrdiff_t offset = 0;
	};

	BackTraceParseState *btps;

	void fillBacktrace(uint32_t backtrace[], uint16_t &backtrace_length, uint32_t PC, uint32_t LR, uint32_t SP) {
		// Fill the backtrace with the PC over time
		// To do this, we need to scan through the dbg data, starting at dbg_data_begin + dbg_symtable_Size
		
		BackTraceParseState local_btps;
		btps = &local_btps; // to get around extraneous lambda types
		local_btps.bt_maxlen = backtrace_length;
		local_btps.bt_len = 1;

		// Scanning for is always the current PC. Start by laoding it into backtrace
		backtrace[0] = PC;
		local_btps.backtrace = backtrace;

		local_btps.PC = PC; local_btps.LR = LR; local_btps.SP = SP;
		local_btps.signalDone = false;

		while (!local_btps.signalDone && local_btps.bt_len < local_btps.bt_maxlen) {
			// Set the initial state of the BTP
			local_btps.parse_state = 0x10;
			local_btps.upper_PC = 0; local_btps.lower_PC = 0; local_btps.unwind_amt = 0;
			// Set the buffer pointer
			local_btps.offset = 0;

			int previous_idx = local_btps.bt_len;
			// Deflate the entire buffer
			tinydeflate::Deflate(+[](){
				return dbg_backtable_begin[btps->offset++];
			}, +[](uint8_t data){ // this lambda is positive. (it forces a cast to pointer to avoid flash space)
				// Our callback; should be called whenever
				
				switch (btps->parse_state & 0xF0) {
					case 0x10:
						btps->lower_PC <<= 8; btps->lower_PC |= data;
						++btps->parse_state;
						if ((btps->parse_state & 0x0F) == 4) btps->parse_state = 0x20;
						break;
					case 0x20:
						btps->upper_PC <<= 8; btps->upper_PC |= data;
						++btps->parse_state;
						if ((btps->parse_state & 0x0F) == 2) {
							btps->upper_PC += btps->lower_PC;
							btps->parse_state = 0x30;
						}
						break;
					case 0x30:
						btps->unwind_amt <<= 8; btps->unwind_amt |= data;
						++btps->parse_state;
						break;
					default:
						break;
				}

				if (btps->parse_state == 0x32) {
					DEBUG_EXCEPTION_PRINT("found bt %08x - %08x = %d; currently at PC=%08x; SP=%08x; LR=%08x\n", btps->lower_PC, btps->upper_PC, btps->unwind_amt, btps->PC, btps->SP, btps->LR);
					// Check if this is an acceptable unwind value.
					while (btps->lower_PC <= btps->PC && btps->PC < btps->upper_PC) {
						DEBUG_EXCEPTION_PRINT("matches in %08x - %08x = %d; currently at PC=%08x; SP=%08x; LR=%08x\n", btps->lower_PC, btps->upper_PC, btps->unwind_amt, btps->PC, btps->SP, btps->LR);
						// It is! apply the unwind amount
						_applyUnwindTo(btps->PC, btps->LR, btps->SP, btps->unwind_amt);
						// Push the new PC to the backtrace
						btps->backtrace[btps->bt_len++] = btps->PC % 2 == 0 ? btps->PC + 1 : btps->PC;
						DEBUG_EXCEPTION_PRINT("now at PC=%08x; SP=%08x; LR=%08x\n", btps->PC, btps->SP, btps->LR);

						// Check if we have reached the end of the backtrace
						// (are we past the end of stack / are we in an invalid addres)
						if (btps->SP >= reinterpret_cast<uint32_t>(&_estack) || btps->PC < 0x0800'0000 || btps->PC > 0x2400'0000) {
							btps->signalDone = true;
							return true;
						}

						// Check if we have overshot our next PC.
						if (btps->PC < btps->lower_PC) {return true;} // If so, return early and re-decompress.

						// If we are still in this unwind-block, this inner loop keeps running.
						// If the value is later, this function returns and decompression continues.
					}

					//DEBUG_EXCEPTION_PRINT("advancing to next\n");


					btps->unwind_amt = 0;
					btps->lower_PC = 0;
					btps->upper_PC = 0;
					btps->parse_state = 0x10; // Get ready for the next value
				}

				return false;
			});

			// If the idx didn't change, we're stuck in an infinite loop; return and break
			if (previous_idx == btps->bt_len) {
				puts("stuck in infinite loop during BT decode; try compiling with -g");

				break;
			}

			//DEBUG_EXCEPTION_PRINT("starting over\n");
		}

		backtrace_length = local_btps.bt_len;
		btps = nullptr;
	}

	struct SymbolParseState {
		uint16_t parse_state; // Upper byte - mode, lower byte - idx.
							 // Mode:
							 //    00 - not init
							 //    01 - reading address
							 //    02 - reading name

		char *currname;
		uint32_t curraddr;

		char *prevname;
		uint32_t prevaddr;

		char namebuf1[64];
		char namebuf2[64];
		ptrdiff_t offset = 0;

		char (*resolved)[64];
		const uint32_t *backtrace;
		uint16_t bt_len;
	};

	SymbolParseState *sps;

	void fillSymbols(char resolved[][64], const uint32_t backtrace[], uint16_t length) {
		// Copy out the symbol table into the resolved table.
		//
		// Work by scanning through the entire table of values with deflate. Start by setting resolved to nulls, if something is a null check if we are greater than it.
		// If so, use the previous value
		
		// Init the SBS
		SymbolParseState local_sps;
		sps = &local_sps;

		sps->parse_state = 0x01'00;
		sps->currname = sps->namebuf1;
		sps->prevname = sps->namebuf2;

		sps->curraddr = 0;
		sps->prevaddr = 0;
		sps->offset   = 0;

		sps->bt_len = length;
		sps->backtrace = backtrace;
		sps->resolved = resolved;

		memset(resolved, 0, length * 64);

		tinydeflate::Deflate(+[](){
			return dbg_symtable_begin[sps->offset++];
		}, +[](uint8_t data){ // this lambda is positive. (it forces a cast to pointer to avoid flash space)
			// Check the state
			switch (sps->parse_state & 0xFF00) {
				case 0x0100:
					{
						sps->curraddr <<= 8; sps->curraddr |= data;
						++sps->parse_state;
						if ((sps->parse_state & 0x0F) == 4) sps->parse_state = 0x0200;
					}
					break;
				case 0x0200:
					{
						// always add the data, so we get the null terminator
						sps->currname[sps->parse_state & 0xFF] = data;
						if (data == 0) {
							DEBUG_EXCEPTION_PRINT("got ai: %s at %04x\n", sps->currname, sps->curraddr);
							// Handle the new data
							
							// ... but first check if we've parsed at least one thing
							
							if (sps->prevaddr != 0) {
								for (int i = 0; i < sps->bt_len; ++i) {
									if ((sps->resolved[i][0] == 0 && sps->backtrace[i] == sps->curraddr) || (sps->backtrace[i] % 2 == 0 && sps->backtrace[i] + 1 == sps->curraddr)) {
										DEBUG_EXCEPTION_PRINT("resolving c2 with %04x at %04x (%s)\n", sps->prevaddr, sps->backtrace[i], sps->currname);
										strcpy(sps->resolved[i], sps->currname);
									}
									else if (sps->resolved[i][0] == 0 && sps->backtrace[i] < sps->curraddr) {
										DEBUG_EXCEPTION_PRINT("resolving c1 with %04x at %04x (%s)\n", sps->prevaddr, sps->backtrace[i], sps->prevname);
										strcpy(sps->resolved[i], sps->prevname);
									}
								}
							}

							// Now set the pointers around
							sps->prevaddr = sps->curraddr;
							auto old_ptr = sps->prevname;
							sps->prevname = sps->currname;
							sps->currname = old_ptr;

							// Go back to initial state
							sps->parse_state = 0x0100;
						}
						else {
							++sps->parse_state;
						}
					}
					break;
				default:
					break;
			}
			
			return false; // Done to avoid making multiple kinds of deflaters.
		});
	}

	void loadRegsFromFaultTrace(uint32_t *faultSP, uint32_t &PC, uint32_t &LR, uint32_t &SP) {
		PC = faultSP[6];
		LR = faultSP[5];
		SP = reinterpret_cast<uint32_t>(faultSP) + 24;
	}

};
