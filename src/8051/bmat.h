#ifndef __BMAT_H__
#define __BMAT_H__
#include <stc/STC12C5630AD.h>

extern void bmat_delay(unsigned int);
#define DEBOUNCE(pin) bmat_delay(10); while(!(pin)); bmat_delay(10)

void resetRows(void);
void resetCols(void);
unsigned char checkCols(void);

sbit ROW1 = P1 ^ 7;
sbit ROW2 = P2 ^ 0;
sbit ROW3 = P2 ^ 1;
sbit ROW4 = P2 ^ 6;
sbit ROW5 = P2 ^ 7;
sbit ROW6 = P3 ^ 7;
sbit COL1 = P2 ^ 2;
sbit COL2 = P2 ^ 3;
sbit COL3 = P3 ^ 0;
sbit COL4 = P3 ^ 1;
sbit COL5 = P3 ^ 2;
sbit COL6 = P3 ^ 3;
sbit COL7 = P3 ^ 4;
sbit COL8 = P3 ^ 5;
sbit COL9 = P2 ^ 4;
sbit COL10 = P2 ^ 5;

#endif
