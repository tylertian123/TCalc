#include <stc/STC12C5630AD.h>
#include "sbdi.h"
#include "adc.h"
#include "keydef.h"

sbit BUTTON = P1 ^ 0;
#define CHANNEL_X_AXIS 1
#define CHANNEL_Y_AXIS 2

void delay (unsigned int a){
	unsigned int i;
	while( --a != 0){
		for(i = 0; i < 600; i++);
	}
}

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

void resetRows() {
	ROW1 = ROW2 = ROW3 = ROW4 = ROW5 = ROW6 = 1;
}
void resetCols() {
	COL1 = COL2 = COL3 = COL4 = COL5 = COL6 = COL7 = COL8 = COL9 = COL10 = 1;
}

const unsigned short MIN_THRESH = 192;
const unsigned short MAX_THRESH = 832;

const unsigned short code KEYMAP_NORMAL[6][10] = {
	{ KEY_LCQ, KEY_LCW, KEY_LCE, KEY_LCR, KEY_LCT, KEY_LCY, KEY_LCU, KEY_LCI, KEY_LCO, KEY_LCP },
	{ KEY_LCA, KEY_LCS, KEY_LCD, KEY_LCF, KEY_LCG, KEY_LCH, KEY_LCJ, KEY_LCK, KEY_LCL, KEY_SPACE },
	{ KEY_SHIFT, KEY_LCZ, KEY_LCX, KEY_LCC, KEY_LCV, KEY_LCB, KEY_LCN, KEY_LCM, KEY_COMMA, KEY_CTRL },
	{ KEY_LBRACKET, KEY_RBRACKET, KEY_7, KEY_8, KEY_9, KEY_0, KEY_PLUS, KEY_MINUS, KEY_DELETE, KEY_ALLCLEAR },
	{ KEY_TRIG, KEY_CAT, KEY_4, KEY_5, KEY_6, KEY_DOT, KEY_MUL, KEY_DIV, KEY_SQUARE, KEY_EXPONENT },
	{ KEY_EXP, KEY_LOG10, KEY_1, KEY_2, KEY_3, KEY_EE, KEY_ANS, KEY_ENTER, KEY_ROOT, KEY_PI },
};
const unsigned short code KEYMAP_SHIFT[6][10] = {
	{ KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P },
	{ KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_CONFIG },
	{ KEY_SHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_CTRL },
	{ KEY_LBRACKET, KEY_RBRACKET, KEY_7, KEY_8, KEY_9, KEY_0, KEY_SUM, KEY_MINUS, KEY_CLEAR, KEY_ALLCLEAR },
	{ KEY_TRIG, KEY_CAT, KEY_4, KEY_5, KEY_6, KEY_DOT, KEY_PRODUCT, KEY_IMPROP, KEY_CUBE, KEY_EXPONENT },
	{ KEY_LN, KEY_LOGN, KEY_1, KEY_2, KEY_3, KEY_EE, KEY_ANS, KEY_APPROX, KEY_NTHROOT, KEY_EULER },
};
const unsigned short code KEYMAP_CTRL[6][10] = {
	{ KEY_LCQ, KEY_LCW, KEY_LCE, KEY_LCR, KEY_LCT, KEY_LCY, KEY_LCU, KEY_LCI, KEY_LCO, KEY_LCP },
	{ KEY_LCA, KEY_LCS, KEY_LCD, KEY_LCF, KEY_GRAPH, KEY_LCH, KEY_LCJ, KEY_LCK, KEY_LCL, KEY_SPACE },
	{ KEY_SHIFT, KEY_LCZ, KEY_LCX, KEY_LCC, KEY_LCV, KEY_LCB, KEY_LCN, KEY_LCM, KEY_COMMA, KEY_CTRL },
	{ KEY_LBRACKET, KEY_RBRACKET, KEY_7, KEY_8, KEY_9, KEY_0, KEY_PLUS, KEY_MINUS, KEY_DELETE, KEY_ALLCLEAR },
	{ KEY_TRIG, KEY_CAT, KEY_4, KEY_5, KEY_6, KEY_DOT, KEY_MUL, KEY_MIXNUM, KEY_SQUARE, KEY_EXPONENT },
	{ KEY_EXP, KEY_LOG10, KEY_1, KEY_2, KEY_3, KEY_EE, KEY_ANS, KEY_EQUAL, KEY_ROOT, KEY_CONST },
};

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
	P1M0 |= 0x06; //0000 0110
	P1M1 &= 0xF9; //1111 1001
	//Turn on ADC power
	ADC_CONTR |= 0x80;
	//Configure ADC
	//1 conversion per 90 CPU cycles
	ADC_CONTR |= 0x60; //0110 0000
	//Clear ADC Flag
	ADC_CONTR &= 0xEF; //1110 1111
	
	BUTTON = 1;
	resetRows();
	resetCols();
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
		
		resetRows();
		
	}
}