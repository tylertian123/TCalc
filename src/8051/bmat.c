#include "bmat.h"

void bmat_delay (unsigned int a){
	unsigned int i;
	while( --a != 0){
		for(i = 0; i < 600; i++);
	}
}

void resetRows() {
	ROW1 = ROW2 = ROW3 = ROW4 = ROW5 = ROW6 = 1;
}
void resetCols() {
	COL1 = COL2 = COL3 = COL4 = COL5 = COL6 = COL7 = COL8 = COL9 = COL10 = 1;
}

unsigned char checkCols() {
	if(!COL1) {
		//DEBOUNCE(COL1);
		return 0;
	}
	if(!COL2) {
		//DEBOUNCE(COL2);
		return 1;
	}
	if(!COL3) {
		//DEBOUNCE(COL3);
		return 2;
	}
	if(!COL4) {
		//DEBOUNCE(COL4);
		return 3;
	}
	if(!COL5) {
		//DEBOUNCE(COL5);
		return 4;
	}
	if(!COL6) {
		//DEBOUNCE(COL6);
		return 5;
	}
	if(!COL7) {
		//DEBOUNCE(COL7);
		return 6;
	}
	if(!COL8) {
		//DEBOUNCE(COL8);
		return 7;
	}
	if(!COL9) {
		//DEBOUNCE(COL9);
		return 8;
	}
	if(!COL10) {
		//DEBOUNCE(COL10);
		return 9;
	}
	return 0xFF;
}
