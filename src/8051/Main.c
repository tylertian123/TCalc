#include <stc/STC12C5630AD.h>
#include <intrins.h>
#include "sbdi.h"

typedef bit bool;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

sbit LED0 = P2 ^ 6;
sbit LED1 = P2 ^ 7;
sbit LED2 = P3 ^ 7;

sbit BUTTON = P1 ^ 2;

void delay (unsigned int a){
	unsigned int i;
	while( --a != 0){
		for(i = 0; i < 600; i++);
	}
}

void ADC_StartConv(uint8_t channel) {
	//Clear ADC Flag
	ADC_CONTR &= 0xEF; //1110 1111
	//Set channel by setting the last 3 bits
	//First clear the bits
	ADC_CONTR &= 0xF8; //1111 1000
	//And then set them
	ADC_CONTR |= channel & 0x07; //0000 0111
	//Start conversion
	ADC_CONTR |= 0x08;
	//Delay 4 clock cycles
	_nop_();
	_nop_();
	_nop_();
	_nop_();
}
bool ADC_ConvFin() {
	return ADC_CONTR & 0x10; //0001 0000
}
uint16_t ADC_GetResult() {
	uint16_t result = 0;
	//Clear ADC Flag
	ADC_CONTR &= 0xEF; //1110 1111
	//Reassemble the result from the scattered bits
	result = ADC_DATA << 2;
	result |= ADC_LOW2;
	//Delay 4 clock cycles
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	return result;
}

void main(void) {
	uint8_t channel = 0;
	uint16_t result = 0;
	delay(1000);
	//Set input pins to high impedance mode
	P1M0 |= 0x03; //0000 0011
	P1M1 &= 0xFC; //1111 1100
	//Turn on ADC power
	ADC_CONTR |= 0x80;
	//Configure ADC
	//1 conversion per 1080 CPU cycles
	ADC_CONTR &= 0x9F; //1001 1111
	//Clear ADC Flag
	ADC_CONTR &= 0xEF; //1110 1111
	
	LED0 = LED1 = LED2 = 0;
	BUTTON = 1;
	while(1) {
		SBDI_BeginTransmission();
		SBDI_SendByte(0xFF);
		SBDI_EndTransmission();
		
		ADC_StartConv(channel);
		while(!ADC_ConvFin());
		result = ADC_GetResult();
		
		if(result >= 768) {
			LED0 = LED1 = LED2 = 1;
		}
		else if(result >= 512) {
			LED0 = LED1 = 1;
			LED2 = 0;
		}
		else if(result >= 256) {
			LED0 = 1;
			LED1 = LED2 = 0;
		}
		else {
			LED0 = LED1 = LED2 = 0;
		}
		
		if(!BUTTON) {
			delay(50);
			channel = channel == 0 ? 1 : 0;
			while(!BUTTON);
			delay(50);
		}
		
//		SBDI_BeginTransmission();
//		SBDI_SendByte(0xFF);
//		SBDI_EndTransmission();
	}
}