#ifndef __BMAT_H__
#define __BMAT_H__
#include <stc/STC12C5630AD.h>

extern void bmat_delay(unsigned int);
#define DEBOUNCE(pin) bmat_delay(10); while(!(pin)); bmat_delay(10)

void resetRows(void);
void resetCols(void);
unsigned char checkCols(void);

__sbit __at(0x97) ROW1;
__sbit __at(0xA0) ROW2;
__sbit __at(0xA1) ROW3;
__sbit __at(0xA6) ROW4;
__sbit __at(0xA7) ROW5;
__sbit __at(0xB7) ROW6;
__sbit __at(0xA2) COL1;
__sbit __at(0xA3) COL2;
__sbit __at(0xB0) COL3;
__sbit __at(0xB1) COL4;
__sbit __at(0xB2) COL5;
__sbit __at(0xB3) COL6;
__sbit __at(0xB4) COL7;
__sbit __at(0xB5) COL8;
__sbit __at(0xA4) COL9;
__sbit __at(0xA5) COL10;

#endif
