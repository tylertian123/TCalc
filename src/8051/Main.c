#include <stc/STC12C5630AD.h>
#include "sbdi.h"
#include "adc.h"
#include "keydef.h"
#include "bmat.h"

sbit BUTTON = P1 ^ 0;
#define CHANNEL_X_AXIS 1
#define CHANNEL_Y_AXIS 2

//Configs
#define MIN_THRESH 192
#define MAX_THRESH 832
#define REPEAT_KEY_DELAY 150
#define HOLD_COUNTER_MAX 80

void delay (unsigned int a){
	unsigned int i;
	while( --a != 0){
		for(i = 0; i < 600; i++);
	}
}

//Keymaps
const unsigned short code KEYMAP_NORMAL[6][10] = {
	{ KEY_LCQ, KEY_LCW, KEY_LCE, KEY_LCR, KEY_LCT, KEY_LCY, KEY_LCU, KEY_LCI, KEY_LCO, KEY_LCP },
	{ KEY_LCA, KEY_LCS, KEY_LCD, KEY_LCF, KEY_LCG, KEY_LCH, KEY_LCJ, KEY_LCK, KEY_LCL, KEY_CONFIG },
	{ KEY_SHIFT, KEY_LCZ, KEY_LCX, KEY_LCC, KEY_LCV, KEY_LCB, KEY_LCN, KEY_LCM, KEY_COMMA, KEY_CTRL },
	{ KEY_LBRACKET, KEY_RBRACKET, KEY_7, KEY_8, KEY_9, KEY_0, KEY_PLUS, KEY_MINUS, KEY_DELETE, KEY_ALLCLEAR },
	{ KEY_TRIG, KEY_CAT, KEY_4, KEY_5, KEY_6, KEY_DOT, KEY_MUL, KEY_DIV, KEY_SQUARE, KEY_EXPONENT },
	{ KEY_EXP, KEY_LOG10, KEY_1, KEY_2, KEY_3, KEY_EE, KEY_ANS, KEY_ENTER, KEY_ROOT, KEY_PI },
};
const unsigned short code KEYMAP_SHIFT[6][10] = {
	{ KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P },
	{ KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SPACE },
	{ KEY_SHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_CTRL },
	{ KEY_LBRACKET, KEY_RBRACKET, KEY_7, KEY_8, KEY_9, KEY_0, KEY_SUM, KEY_MINUS, KEY_CLEAR, KEY_ALLCLEAR },
	{ KEY_TRIG, KEY_RECALL, KEY_4, KEY_5, KEY_6, KEY_DOT, KEY_PRODUCT, KEY_FRAC, KEY_CUBE, KEY_EXPONENT },
	{ KEY_LN, KEY_LOGN, KEY_1, KEY_2, KEY_3, KEY_EE, KEY_ANS, KEY_APPROX, KEY_NTHROOT, KEY_EULER },
};
const unsigned short code KEYMAP_CTRL[6][10] = {
	{ KEY_LCQ, KEY_LCW, KEY_LCE, KEY_LCR, KEY_LCT, KEY_LCY, KEY_LCU, KEY_LCI, KEY_LCO, KEY_LCP },
	{ KEY_LCA, KEY_LCS, KEY_LCD, KEY_LCF, KEY_GRAPH, KEY_LCH, KEY_LCJ, KEY_LCK, KEY_LCL, KEY_CONFIG },
	{ KEY_SHIFT, KEY_LCZ, KEY_LCX, KEY_LCC, KEY_LCV, KEY_LCB, KEY_LCN, KEY_LCM, KEY_COMMA, KEY_CTRL },
	{ KEY_LBRACKET, KEY_RBRACKET, KEY_7, KEY_8, KEY_9, KEY_0, KEY_PLUS, KEY_MINUS, KEY_CLEARVAR, KEY_ALLCLEAR },
	{ KEY_TRIG, KEY_CAT, KEY_4, KEY_5, KEY_6, KEY_DOT, KEY_MUL, KEY_FRACFMT, KEY_SQUARE, KEY_EXPONENT },
	{ KEY_EXP, KEY_LOG10, KEY_1, KEY_2, KEY_3, KEY_EE, KEY_ANS, KEY_EQUAL, KEY_ROOT, KEY_CONST },
};

bit shift = 0;
bit ctrl = 0;

sbit STATUS = P1 ^ 3;

void sendKey(unsigned short key) {
	STATUS = !STATUS;
	SBDI_BeginTransmission();
	SBDI_SendByte(key >> 8);
	SBDI_SendByte(key & 0x00FF);
	SBDI_EndTransmission();
}
void send(unsigned char row, unsigned char col) {
	//Handle shift and ctrl keys separately
	if(KEYMAP_NORMAL[row][col] == KEY_SHIFT) {
		shift = !shift;
		sendKey(shift ? KEY_SHIFTON : KEY_SHIFTOFF);
		return;
	}
	else if(KEYMAP_NORMAL[row][col] == KEY_CTRL) {
		ctrl = !ctrl;
		sendKey(ctrl ? KEY_CTRLON : KEY_CTRLOFF);
		return;
	}
	
	//Check if shift and ctrl keys are down
	if(shift) {
		//Send the key, delay and deactivate shift and ctrl
		sendKey(KEYMAP_SHIFT[row][col]);
		delay(10);
		sendKey(KEY_SHIFTOFF);
		shift = 0;
	}
	else if(ctrl) {
		sendKey(KEYMAP_CTRL[row][col]);
		delay(10);
		sendKey(KEY_CTRLOFF);
		ctrl = 0;
	}
	else {
		sendKey(KEYMAP_NORMAL[row][col]);
	}
}
void checkAndSend(unsigned char row) {
	unsigned char col = checkCols();
	unsigned char holdCounter = 0;
	if(col == 0xFF) {
		return;
	}
	
	//Key hold checker same logic as joystick's
	send(row, col);
	while(checkCols() == col) {
		if(holdCounter <= HOLD_COUNTER_MAX) {
			holdCounter ++;
			delay(10);
		}
		else {
			send(row, col);
			delay(REPEAT_KEY_DELAY);
		}
	}
}

void main(void) {
	unsigned short result = 0;
	unsigned char holdCounter = 0;
	
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
		result = ADC_SyncConv(CHANNEL_X_AXIS);
		//Check if the value is below the min threshold, and that the last time we checked it was above
		if(result < MIN_THRESH) {
			//First send the key
			sendKey(KEY_LEFT);
			//Then check if the stick is held down
			while(ADC_SyncConv(CHANNEL_X_AXIS) < MIN_THRESH) {
				//While holding down and the counter does not exceed the max, increment the counter and delay
				if(holdCounter <= HOLD_COUNTER_MAX) {
					holdCounter ++;
					delay(10);
				}
				//Otherwise send the key and delay
				else {
					sendKey(KEY_LEFT);
					delay(REPEAT_KEY_DELAY);
				}
			}
			holdCounter = 0;
		}
		else if(result > MAX_THRESH) {
			sendKey(KEY_RIGHT);
			while(ADC_SyncConv(CHANNEL_X_AXIS) > MAX_THRESH) {
				if(holdCounter <= HOLD_COUNTER_MAX) {
					holdCounter ++;
					delay(10);
				}
				else {
					sendKey(KEY_RIGHT);
					delay(REPEAT_KEY_DELAY);
				}
			}
			holdCounter = 0;
		}
		
		//Check for up and down
		result = ADC_SyncConv(CHANNEL_Y_AXIS);
		if(result < MIN_THRESH) {
			sendKey(KEY_UP);
			while(ADC_SyncConv(CHANNEL_Y_AXIS) < MIN_THRESH) {
				if(holdCounter <= HOLD_COUNTER_MAX) {
					holdCounter ++;
					delay(10);
				}
				else {
					sendKey(KEY_UP);
					delay(REPEAT_KEY_DELAY);
				}
			}
			holdCounter = 0;
		}
		else if(result > MAX_THRESH) {
			sendKey(KEY_DOWN);
			while(ADC_SyncConv(CHANNEL_Y_AXIS) > MAX_THRESH) {
				if(holdCounter <= HOLD_COUNTER_MAX) {
					holdCounter ++;
					delay(10);
				}
				else {
					sendKey(KEY_DOWN);
					delay(REPEAT_KEY_DELAY);
				}
			}
			holdCounter = 0;
		}
		
		if(!BUTTON) {
			DEBOUNCE(BUTTON);
			sendKey(KEY_CENTER);
		}
		
		resetRows();
		ROW1 = 0;
		checkAndSend(0);
		resetRows();
		ROW2 = 0;
		checkAndSend(1);
		resetRows();
		ROW3 = 0;
		checkAndSend(2);
		resetRows();
		ROW4 = 0;
		checkAndSend(3);
		resetRows();
		ROW5 = 0;
		checkAndSend(4);
		resetRows();
		ROW6 = 0;
		checkAndSend(5);
		resetRows();
	}
}