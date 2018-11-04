#include <stc/STC12C5630AD.h>
#include "sbdi.h"
#include "adc.h"

sbit BUTTON = P1 ^ 2;
#define CHANNEL_X_AXIS 0
#define CHANNEL_Y_AXIS 1

void delay (unsigned int a){
	unsigned int i;
	while( --a != 0){
		for(i = 0; i < 600; i++);
	}
}

const unsigned short MIN_THRESH = 192;
const unsigned short MAX_THRESH = 832;

#define KEY_NULL	0x0000
#define KEY_UP		0x0001
#define KEY_DOWN	0x0002
#define KEY_LEFT	0x0003
#define KEY_RIGHT	0x0004
#define KEY_CENTER	0x0005

void sendKey(unsigned short key) {
	SBDI_BeginTransmission();
	SBDI_SendByte(key >> 8);
	SBDI_SendByte(key & 0x00FF);
	SBDI_EndTransmission();
}

void main(void) {
	unsigned short result = 0;
	unsigned short lastXResult = 512;
	unsigned short lastYResult = 512;
	
	delay(1000);
	//Set input pins to high impedance mode
	P1M0 |= 0x03; //0000 0011
	P1M1 &= 0xFC; //1111 1100
	//Turn on ADC power
	ADC_CONTR |= 0x80;
	//Configure ADC
	//1 conversion per 90 CPU cycles
	ADC_CONTR |= 0x60; //0110 0000
	//Clear ADC Flag
	ADC_CONTR &= 0xEF; //1110 1111
	
	BUTTON = 1;
	while(1) {
		//Check for left and right
		ADC_StartConv(CHANNEL_X_AXIS);
		while(!ADC_ConvFin());
		result = ADC_GetResult();
		//Check if the value is below the min threshold, and that the last time we checked it was above
		if(result < MIN_THRESH && lastXResult >= MIN_THRESH) {
			sendKey(KEY_LEFT);
			delay(20);
		}
		else if(result > MAX_THRESH && lastXResult <= MAX_THRESH) {
			sendKey(KEY_RIGHT);
			delay(20);
		}
		lastXResult = result;
		
		//Check for up and down
		ADC_StartConv(CHANNEL_Y_AXIS);
		while(!ADC_ConvFin());
		result = ADC_GetResult();
		if(result < MIN_THRESH && lastYResult >= MIN_THRESH) {
			sendKey(KEY_UP);
			delay(20);
		}
		else if(result > MAX_THRESH && lastYResult <= MAX_THRESH) {
			sendKey(KEY_DOWN);
			delay(20);
		}
		lastYResult = result;
		
		if(!BUTTON) {
			delay(10);
			sendKey(KEY_CENTER);
			while(!BUTTON);
			delay(10);
		}
	}
}