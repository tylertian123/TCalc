#include "sbdi.h"
#include <stc/STC12C5630AD.h>

#define SBDI_CLK_DELAY 150

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

void SBDI_SendByte(uint8_t b) {
	uint8_t mask = 0x80;
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

bit SBDI_ReceivePending() {
	return !EN;
}

uint32_t SBDI_ReceiveBuffer = 0;

void SBDI_Receive() {
	bit transmissionFailed = 0;
	bit parity = 0;
	bit dat;

	uint8_t bitCounter = 0;

	while(!EN) {
		// Pull clock low
		CLK = 0;
		delay_cycles(SBDI_CLK_DELAY);
		// Pull clock high
		CLK = 1;
		dat = DAT;
		delay_cycles(SBDI_CLK_DELAY);
		if(transmissionFailed) {
			continue;
		}

		parity ^= dat;
		bitCounter ++;

		if(bitCounter == 9) {
			if(parity) {
				transmissionFailed = 1;
				SBDI_ReceiveBuffer = 0;
			}

			parity = 0;
			bitCounter = 0;
		}
		else {
			SBDI_ReceiveBuffer <<= 1;
			SBDI_ReceiveBuffer |= dat;
		}
	}
}
