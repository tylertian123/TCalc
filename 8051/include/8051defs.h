#ifndef __8051_DEFS_H__
#define __8051_DEFS_H__

#ifndef __VSCODE

#define bit __bit
#define code __code
#define data __data
#define xdata __xdata

#define SBIT(name, addr) __sbit __at(addr) name
#define SFR(name, addr) __sfr __at(addr) name

#define ASM(asm) __asm asm __endasm

#else

#define bit int
#define code
#define data
#define xdata

#define SBIT(name, addr) int name
#define SFR(name, addr) unsigned char name

#define ASM(asm)

#endif

#endif
