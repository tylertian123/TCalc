#include "adc.h"
#include <stc/STC12C5630AD.h>

void ADC_StartConv(unsigned char channel) {
	// Clear ADC Flag
	ADC_CONTR &= 0xEF; // 1110 1111
	// Set channel by setting the last 3 bits
	// First clear the bits
	ADC_CONTR &= 0xF8; // 1111 1000
	// And then set them
	ADC_CONTR |= channel & 0x07; // 0000 0111
	// Start conversion
	ADC_CONTR |= 0x08;
	// Delay 4 clock cycles
	ASM(nop);
	ASM(nop);
	ASM(nop);
	ASM(nop);
}

	
unsigned short ADC_GetResult() {
	unsigned short result = 0;
	// Clear ADC Flag
	ADC_CONTR &= 0xEF; // 1110 1111
	// Reassemble the result from the scattered bits
	result = ADC_DATA << 2;
	result |= ADC_LOW2;
	// Delay 4 clock cycles
	__asm nop __endasm;
	__asm nop __endasm;
	__asm nop __endasm;
	__asm nop __endasm;
	return result;
}

bit ADC_ConvFin() {
	return ADC_CONTR & 0x10;
}

unsigned short ADC_SyncConv(unsigned char channel) {
	ADC_StartConv(channel);
	while(!ADC_ConvFin()) {}
	return ADC_GetResult();
}
