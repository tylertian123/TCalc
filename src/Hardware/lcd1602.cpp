#include "lcd1602.hpp"

namespace lcd {
	
	bool LCD1602::clear() {
		return writeCommand(Command::CLEAR);
	}
	bool LCD1602::home() {
		return writeCommand(Command::HOME);
	}
	
	bool LCD1602::init() {
		if(!writeCommand(Command::EIGHT_BIT_TWO_LINES)) {
			return false;
		}
		if(!writeCommand(Command::DISPLAY_ON_CURSOR_OFF)) {
			return false;
		}
		if(!writeCommand(Command::ENTRY_CURSOR_SHIFT_RIGHT)) {
			return false;
		}
		if(!home()) {
			return false;
		}
		if(!clear()) {
			return false;
		}
		return true;
	}
	
	bool LCD1602::setCursor(uint8_t row, uint8_t col) {
		if(row != 0) {
			col += 0x40;
		}
		return writeCommand(0x80 | col);
	}
}
