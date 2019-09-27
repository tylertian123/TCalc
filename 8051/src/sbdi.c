#include "sbdi.h"
#include <stc/STC12C5630AD.h>

SBIT(EN, 0x96);
SBIT(CLK, 0x95);
SBIT(DAT, 0x94);

void delay_cycles(unsigned int a) {
	while(a--);
}

void SBDI_BeginTransmission() {
	DAT = 1;
	EN = 0;
}

void SBDI_EndTransmission() {
	EN = 1;
	DAT = 1;
}

void SBDI_SendByte(unsigned char b) {
	unsigned char mask = 0x80;
    // Use even parity
	bit parity = 0;
	bit dat;
	do {
		dat = b & mask;
        // Clock is active low
        // Wait until clock is low
		while(CLK);
        DAT = dat;
        // Wait until clock is high again
        while(!CLK);
		parity ^= dat;
	} while(mask >>= 1);
    // Send parity bit
	while(CLK);
    DAT = parity;
    while(!CLK);
}
