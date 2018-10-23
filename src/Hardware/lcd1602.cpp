#include "lcd1602.hpp"
namespace lcd {
	
	#define W_CMD(x) \
		if(!writeCommand(x)) \
			return false
	
	bool LCD1602::clear() {
		return writeCommand(Command::CLEAR);
	}
	bool LCD1602::home() {
		return writeCommand(Command::HOME);
	}
	
	bool LCD1602::init() {
		if(!FOUR_WIRE_INTERFACE) {
			delay::ms(15);
			writeCommandNoWait(Command::EIGHT_BIT_TWO_LINES);
			delay::ms(5);
			writeCommandNoWait(Command::EIGHT_BIT_TWO_LINES);
			delay::ms(1);
			writeCommandNoWait(Command::EIGHT_BIT_TWO_LINES);
		}
		else {
			delay::ms(15);
			writeCommandNoWait(Command::FOUR_BIT_TWO_LINES);
			delay::ms(5);
			writeCommandNoWait(Command::FOUR_BIT_TWO_LINES);
			delay::ms(1);
			writeCommandNoWait(Command::FOUR_BIT_TWO_LINES);
		}
		
		W_CMD(Command::DISPLAY_OFF);
		W_CMD(Command::CLEAR);
		W_CMD(Command::ENTRY_CURSOR_SHIFT_RIGHT);
		W_CMD(Command::DISPLAY_ON_CURSOR_OFF);
		return true;
	}
	
	bool LCD1602::setCursor(uint8_t row, uint8_t col) {
		if(row != 0) {
			col += 0x40;
		}
		return writeCommand(0x80 | col);
	}
}
