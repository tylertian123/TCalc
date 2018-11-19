#include "lcd12864.hpp"
#include "util.hpp"

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
		
		for(uint8_t row = 0; row < 32; row ++) {
			for(uint8_t col = 0; col < 16; col ++) {
				//The row gets written first
				//There are 32 rows (bottom 32 are just extensions of the top 32)
				//And then the column gets written (16 pixels)
				W_CMD(0x80 | row);
				W_CMD(0x80 | col);
				W_CHR(0x00);
				W_CHR(0x00);
				
				//Clear our buffers
				drawBuf[row][col] = 0x0000;
				dispBuf[row][col] = 0x0000;
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
				if(dispBuf[row][col] != drawBuf[row][col]) {
					//Update the display buffer
					dispBuf[row][col] = drawBuf[row][col];
					W_CMD(0x80 | row);
					W_CMD(0x80 | col);
					//Write higher order byte first
					W_CHR(dispBuf[row][col] >> 8);
					W_CHR(dispBuf[row][col] & 0x00FF);
				}
			}
		}
		return true;
	}
	
	void LCD12864::clearDrawingBuffer() {
		std::memset(drawBuf, 0, sizeof(drawBuf));
	}
	
	void LCD12864::setPixel(int16_t x, int16_t y, bool state) {
		if(x >= 128 || y >= 64 || x < 0 || y < 0) {
			return;
		}
		
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
			drawBuf[row][col] |= mask;
		}
		else {
			mask = ~(0x8000 >> offset); //Same as above, only flip the 1s and 0s
			drawBuf[row][col] &= mask;
		}
	}
	
	void LCD12864::ORDrawBufferByte(uint16_t x, uint16_t y, uint8_t data) {
		if(x >= 16 || y >= 64) {
			return;
		}
		//Calculate row and column
		//The row is just the Y if y < 32, otherwise it's y - 32
		uint8_t row = y < 32 ? y : y - 32;
		//The column is just x / 2 if y < 32, otherwise it's x / 2 + 8
		uint8_t col = x / 2 + (y < 32 ? 0 : 8);
		//If x is even, then the byte is on the left of the uint16, so left shift by 8.
		drawBuf[row][col] |= x % 2 == 0 ? data << 8 : data;
	}

    void LCD12864::ANDDrawBufferByte(uint16_t x, uint16_t y, uint8_t data) {
        if(x >= 16 || y >= 64) {
            return;
        }
        //Calculate row and column
        //The row is just the Y if y < 32, otherwise it's y - 32
        uint8_t row = y < 32 ? y : y - 32;
        //The column is just x / 2 if y < 32, otherwise it's x / 2 + 8
        uint8_t col = x / 2 + (y < 32 ? 0 : 8);
        //If x is even, then the byte is on the left of the uint16, so left shift by 8.
        drawBuf[row][col] &= x % 2 == 0 ? data << 8 : data;
    }
	
	void LCD12864::drawImage(int16_t x, int16_t y, const LCD12864Image &img) {
        //Check for out of bounds
        if(x >= 128 || y >= 64) {
            return;
        }
        if(x + img.width < 0 || y + img.height < 0) {
            return;
        }

		int16_t baseByte = floorDiv(x, static_cast<int16_t>(8));
        int8_t offset = positiveMod(x, static_cast<int16_t>(8));
		for(int16_t row = 0; row < img.height; row ++) {
			//If the byte we're drawing into is out of bounds vertically then break this outer loop
			if(row + y >= 64) {
				break;
			}
            //If the byte is too high up, skip this row
            if(row + y < 0) {
                continue;
            }
			for(int16_t byte = 0; byte < img.bytesWide; byte ++) {
				//If the byte we're drawing into is out of bounds horizontally then break this inner loop
				if(baseByte + byte >= 16) {
					break;
				}
                //If the byte we're drawing into is too far left, skip it
                if(baseByte + byte < 0) {
                    continue;
                }
				
				//The bytes have to be shifted
				uint8_t currentByte = img.data[row * img.bytesWide + byte] >> offset;
				/*
				 * If this byte isn't the first byte, then add the truncated parts of the previous byte to it
				 * Ex. 1111 1010, 1100 0000 right shifted 2:
				 * First byte is just 1111 1010 >> 2 = 0011 1110
				 * Second byte:
				 * 		1100 0000 >> 2 = 0011 0000
				 * 	OR	1111 1010 << 6 = 1000 0000
				 * -------------------------------
				 *						 1011 0000
				 * Final Result: 0011 1110, 1011 0000
				 */
				
				if(byte != 0) {
					currentByte |= img.data[row * img.bytesWide + byte - 1] << (8 - offset);
				}
				
				ORDrawBufferByte(baseByte + byte, row + y, currentByte);
			}
			//Finally, if we shifted by more than one bit, then there must be some bits clipped in the end
			//Here we recover those lost bits and write them to the buffer
			if(offset != 0) {
				uint8_t finalByte = img.data[row * img.bytesWide + img.bytesWide - 1] << (8 - offset);
				ORDrawBufferByte(baseByte + img.bytesWide, row + y, finalByte);
			}
		}
	}
	
	//Bresenham's Line Algorithm
	void drawLineLow(LCD12864 &dest, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
		bool flip = y2 < y1;
		if (flip) {
			y2 = y1 + (y1 - y2);
		}
		dest.setPixel(x1, y1, true);
		uint16_t dx = x2 - x1;
		uint16_t dy = y2 - y1;
		uint16_t dy2 = 2 * dy;
		uint16_t dydx2 = dy2 - 2 * dx;

		int16_t p = dy2 - dx;
		uint16_t y = y1;
		for (uint16_t x = x1 + 1; x <= x2; x++) {
			if (p < 0) {
				dest.setPixel(x, y, true);
				p += dy2;
			}
			else {
				dest.setPixel(x, flip ? --y : ++y, true);
				p += dydx2;
			}

		}
	}
	void drawLineHigh(LCD12864 &dest, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
		uint16_t temp = x1;
		x1 = y1;
		y1 = temp;
		temp = x2;
		x2 = y2;
		y2 = temp;

		bool flip = y2 < y1;
		if (flip) {
			y2 = y1 + (y1 - y2);
		}

		dest.setPixel(y1, x1, true);
		uint16_t dx = x2 - x1;
		uint16_t dy = y2 - y1;
		uint16_t dy2 = 2 * dy;
		uint16_t dydx2 = dy2 - 2 * dx;

		int16_t p = dy2 - dx;
		uint16_t y = y1;
		for (uint16_t x = x1 + 1; x <= x2; x++) {
			if (p < 0) {
				dest.setPixel(y, x, true);
				p += dy2;
			}
			else {
				dest.setPixel(flip ? y -- : y++, x, true);
				p += dydx2;
			}

		}
	}
	void LCD12864::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
		if (abs(y2 - y1) <= abs(x2 - x1)) {
			if (x2 > x1) {
				drawLineLow(*this, x1, y1, x2, y2);
			}
			else {
				drawLineLow(*this, x2, y2, x1, y1);
			}
		}
		else {
			if (y2 > y1) {
				drawLineHigh(*this, x1, y1, x2, y2);
			}
			else {
				drawLineHigh(*this, x2, y2, x1, y1);
			}
		}
	}
	
	#undef W_CMD
	#undef W_CHR
}
