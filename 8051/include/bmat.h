#ifndef __BMAT_H__
#define __BMAT_H__
#include <stc/STC12C5630AD.h>

extern void bmat_delay(unsigned int);
#define DEBOUNCE(pin) bmat_delay(10); while(!(pin)); bmat_delay(10)

void resetRows(void);
void resetCols(void);
unsigned char checkCols(void);

SBIT(ROW1, 0x97);
SBIT(ROW2, 0xA0);
SBIT(ROW3, 0xA1);
SBIT(ROW4, 0xA6);
SBIT(ROW5, 0xA7);
SBIT(ROW6, 0xB7);
SBIT(COL1, 0xA2);
SBIT(COL2, 0xA3);
SBIT(COL3, 0xB0);
SBIT(COL4, 0xB1);
SBIT(COL5, 0xB2);
SBIT(COL6, 0xB3);
SBIT(COL7, 0xB4);
SBIT(COL8, 0xB5);
SBIT(COL9, 0xA4);
SBIT(COL10, 0xA5);

#endif
