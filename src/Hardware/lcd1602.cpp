#include "lcd1602.hpp"
#include "usart.hpp"
namespace lcd {
	
	bool LCD1602::clear() {
		return writeCommand(Command::CLEAR);
	}
	bool LCD1602::home() {
		return writeCommand(Command::HOME);
	}
	
	void LCD1602::init() {
//		if(!writeCommand(Command::EIGHT_BIT_TWO_LINES)) {
//			return false;
//		}
//		if(!writeCommand(Command::DISPLAY_ON_CURSOR_OFF)) {
//			return false;
//		}
//		if(!writeCommand(Command::ENTRY_CURSOR_SHIFT_RIGHT)) {
//			return false;
//		}
//		if(!home()) {
//			return false;
//		}
//		if(!clear()) {
//			return false;
//		}
//		return true;
		delay::ms(15);
		writeCommandNoWait(Command::EIGHT_BIT_TWO_LINES);
		delay::ms(5);
		writeCommandNoWait(Command::EIGHT_BIT_TWO_LINES);
		delay::ms(1);
		writeCommandNoWait(Command::EIGHT_BIT_TWO_LINES);
		delay::ms(1);
		writeCommand(Command::EIGHT_BIT_TWO_LINES);
		delay::ms(1);
		writeCommand(Command::DISPLAY_OFF);
		delay::ms(1);
		writeCommand(Command::CLEAR);
		delay::ms(1);
		writeCommand(Command::ENTRY_CURSOR_SHIFT_RIGHT);
		delay::ms(1);
		writeCommand(Command::DISPLAY_ON_CURSOR_BLINK);
		delay::ms(1);
	}
	
	bool LCD1602::setCursor(uint8_t row, uint8_t col) {
		if(row != 0) {
			col += 0x40;
		}
		return writeCommand(0x80 | col);
	}
}
