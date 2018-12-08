#include "lcd12864_charset.hpp"

namespace lcd {
	uint8_t asciiToIndex(char c) {
		//If char is outside the range, return 0x7F (CHAR_UNKNOWN) minus the offset
		if(c < LCD_CHARSET_LOWBOUND || c >= 0x7F) {
			return 0x7F - LCD_CHARSET_LOWBOUND;
		}
		//Otherwise subtract the offset
		return c - LCD_CHARSET_LOWBOUND;
	}
	const Img& getChar(char c) {
		return *CHAR_ASCII[asciiToIndex(c)];
	}
}
