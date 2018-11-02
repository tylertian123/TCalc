#include "lcd12864_charset.hpp"

namespace lcd {
	uint8_t asciiToIndex(char c) {
		//If char is outside the range, return 0x7F (CHAR_UNKNOWN) minus the offset
		if(c < 0x20 || c >= 0x7F) {
			return 0x7F - 0x20;
		}
		//Otherwise subtract the offset
		return c - 0x20;
	}
	const Img& getChar(char c) {
		return CHAR_ASCII[asciiToIndex(c)];
	}
	const Img& getNumber(uint8_t n) {
		//If the digit requested is outside the limits, return CHAR_UNKNOWN
		if(n >= 10) {
			return CHAR_ASCII[0x7F - 0x20];
		}
		return CHAR_NUMBERS[n];
	}
}
