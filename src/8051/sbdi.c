#include "SBDI.h"
#include <stc/STC12C5630AD.h>

#define SBDI_CLK_DELAY 1200
#define SBDI_EN_DELAY 3000

sbit EN = P1 ^ 6;
sbit CLK = P1 ^ 5;
sbit DAT = P1 ^ 4;

void delay_cycles(unsigned int a) {
	while(a--);
}

void SBDI_BeginTransmission() {
	CLK = 1;
	DAT = 1;
	EN = 0;
    delay_cycles(SBDI_EN_DELAY);
}

void SBDI_EndTransmission() {
	EN = 1;
	CLK = 1;
	DAT = 1;
}

void SBDI_SendSingleBit(bit b) {
	DAT = b;
	CLK = 0;
	delay_cycles(SBDI_CLK_DELAY);
	CLK = 1;
	delay_cycles(SBDI_CLK_DELAY);
}

void SBDI_SendByte(unsigned char b) {
	unsigned char mask = 0x80;
    // Use even parity
	bit parity = 0;
	bit dat;
	do {
		dat = b & mask;
		SBDI_SendSingleBit(dat);
		parity ^= dat;
	} while(mask >>= 1);
	SBDI_SendSingleBit(parity);
}
