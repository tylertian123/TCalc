#include "lcd12864.hpp"

namespace lcd {
	
	#define W_CMD(x) \
		if(!writeCommand(x)) \
			return false
	#define W_CHR(x) \
		if(!writeData(x)) \
			return false
	
	bool LCD12864::clear() {
		return writeCommand(Command::CLEAR);
	}
	bool LCD12864::home() {
		return writeCommand(Command::HOME);
	}
			
	bool LCD12864::init() {
		if(!FOUR_WIRE_INTERFACE) {
			delay::ms(15);
			writeCommandNoWait(Command::NORMAL_CMD_8BIT);
			delay::ms(5);
			writeCommandNoWait(Command::NORMAL_CMD_8BIT);
			delay::ms(5);
		}
		else {
			delay::ms(15);
			writeCommandNoWait(Command::NORMAL_CMD_4BIT);
			delay::ms(5);
			writeCommandNoWait(Command::NORMAL_CMD_4BIT);
			delay::ms(5);
		}
		
		W_CMD(Command::ENTRY_CURSOR_SHIFT_RIGHT);
		W_CMD(Command::CLEAR);
		W_CMD(Command::DISPLAY_ON_CURSOR_OFF);
		return true;
	}
	
	bool LCD12864::setCursor(uint8_t row, uint8_t col) {
		//If using extended command set, first set to use basic, write the address and change back
		if(isUsingExtended()) {
			if(!useBasic()) 
				return false;
			switch(row){
			case 0: col += 0x80; break;
			case 1: col += 0x90; break;
			case 2: col += 0x88; break;
			case 3: col += 0x98; break;
			default: break;
			}
			//Make the first bit 1 and second bit 0 to match the command requirements
			col |= 0x80; //1000 0000
			col &= 0xBF; //1011 1111
			W_CMD(col);
			return useExtended();
		}
		else {
			switch(row){
			case 0: col += 0x80; break;
			case 1: col += 0x90; break;
			case 2: col += 0x88; break;
			case 3: col += 0x98; break;
			default: break;
			}
			//Make the first bit 1 and second bit 0 to match the command requirements
			col |= 0x80; //1000 0000
			col &= 0xBF; //1011 1111
			W_CMD(col);
			return true;
		}
	}
	
	bool LCD12864::isUsingExtended() {
		return extendedCmd;
	}
	bool LCD12864::useExtended() {
		if(extendedCmd) 
			return true;
		W_CMD(FOUR_WIRE_INTERFACE ? Command::EXT_CMD_4BIT : Command::EXT_CMD_8BIT);
		extendedCmd = true;
		return true;
	}
	bool LCD12864::useBasic() {
		if(!extendedCmd)
			return true;
		W_CMD(FOUR_WIRE_INTERFACE ? Command::NORMAL_CMD_4BIT : Command::NORMAL_CMD_8BIT);
		extendedCmd = false;
		return true;
	}
	
	bool LCD12864::isDrawing() {
		return drawing;
	}
	bool LCD12864::startDraw() {
		if(!isUsingExtended()) {
			if(!useExtended()) {
				return false;
			}
		}
		W_CMD(FOUR_WIRE_INTERFACE ? Command::EXT_GRAPHICS_ON_4BIT : Command::EXT_GRAPHICS_ON_8BIT);
		drawing = true;
		return true;
	}
	bool LCD12864::endDraw() {
		W_CMD(FOUR_WIRE_INTERFACE ? Command::EXT_GRAPHICS_OFF_4BIT : Command::EXT_GRAPHICS_OFF_8BIT);
		drawing = false;
		return true;
	}
	
	bool LCD12864::clearDrawing() {
		if(!isDrawing()) {
			return false;
		}
		for(uint8_t i = 0; i < 32; i ++) {
			for(uint8_t j = 0; j < 16; j ++) {
				//The row gets written first
				//There are 32 rows (bottom 32 are just extensions of the top 32)
				//And then the column gets written (16 pixels)
				W_CMD(0x80 | i);
				W_CMD(0x80 | j);
				W_CHR(0x00);
				W_CHR(0x00);
				
				//Clear our buffers
				drawBuf[j][i] = 0x0000;
				dispBuf[j][i] = 0x0000;
			}
		}
		return true;
	}
	//This function takes the drawing buffer, compares it with the display buffer and writes any necessary bytes.
	bool LCD12864::updateDrawing() {
		if(!isDrawing()) {
			return false;
		}
		for(uint8_t row = 0; row < 32; row ++) {
			for(uint8_t col = 0; col < 16; col ++) {
				//Compare drawBuf with dispBuf
				if(dispBuf[col][row] != drawBuf[col][row]) {
					//Update the display buffer
					dispBuf[col][row] = drawBuf[col][row];
					W_CMD(0x80 | row);
					W_CMD(0x80 | col);
					//Write higher order byte first
					W_CHR(dispBuf[col][row] >> 8);
					W_CHR(dispBuf[col][row] & 0x00FF);
				}
			}
		}
		return true;
	}
	
	void LCD12864::setPixel(uint8_t x, uint8_t y, bool state) {
		//Calculate the index of the uint16_t
		uint8_t col = x / 16;
		uint8_t row = y;
		//The LCD has a weird coordinate system; the bottom 32 rows are just extensions of the top 32 rows.
		//So our row is more than 32, subtract 32 and shift the column instead
		if(row >= 32) {
			row -= 32;
			col += 8;
		}
		
		uint8_t offset = x % 16;
		uint16_t mask;
		if(state) {
			mask = 0x8000 >> offset; //1000 0000 0000 0000
			drawBuf[col][row] |= mask;
		}
		else {
			mask = ~(0x8000 >> offset); //Same as above, only flip the 1s and 0s
			drawBuf[col][row] &= mask;
		}
	}
	
	#undef W_CMD
	#undef W_CHR
}
