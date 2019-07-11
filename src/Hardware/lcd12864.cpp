#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "util.hpp"

namespace lcd {
	
	void LCD12864::clear() {
		writeCommand(Command::CLEAR);
	}
	void LCD12864::home() {
		writeCommand(Command::HOME);
	}
			
	void LCD12864::init() {
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
		
		writeCommand(Command::ENTRY_CURSOR_SHIFT_RIGHT);
		writeCommand(Command::CLEAR);
		writeCommand(Command::DISPLAY_ON_CURSOR_OFF);
	}
	
	void LCD12864::setCursor(uint8_t row, uint8_t col) {
		// If using extended command set, first set to use basic, write the address and change back
		if(isUsingExtended()) {
            useBasic();
			switch(row){
			case 0: col += 0x80; break;
			case 1: col += 0x90; break;
			case 2: col += 0x88; break;
			case 3: col += 0x98; break;
			default: break;
			}
			// Make the first bit 1 and second bit 0 to match the command requirements
			col |= 0x80; // 1000 0000
			col &= 0xBF; // 1011 1111
			writeCommand(col);
			useExtended();
		}
		else {
			switch(row){
			case 0: col += 0x80; break;
			case 1: col += 0x90; break;
			case 2: col += 0x88; break;
			case 3: col += 0x98; break;
			default: break;
			}
			// Make the first bit 1 and second bit 0 to match the command requirements
			col |= 0x80; // 1000 0000
			col &= 0xBF; // 1011 1111
			writeCommand(col);
		}
	}
	
	bool LCD12864::isUsingExtended() {
		return extendedCmd;
	}
	void LCD12864::useExtended() {
		if(extendedCmd) 
			return;
		writeCommand(FOUR_WIRE_INTERFACE ? Command::EXT_CMD_4BIT : Command::EXT_CMD_8BIT);
		extendedCmd = true;
	}
	void LCD12864::useBasic() {
		if(!extendedCmd)
			return;
		writeCommand(FOUR_WIRE_INTERFACE ? Command::NORMAL_CMD_4BIT : Command::NORMAL_CMD_8BIT);
		extendedCmd = false;
	}
	
	bool LCD12864::isDrawing() {
		return drawing;
	}
	void LCD12864::startDraw() {
		if(!isUsingExtended()) {
            useExtended();
		}
		writeCommand(FOUR_WIRE_INTERFACE ? Command::EXT_GRAPHICS_ON_4BIT : Command::EXT_GRAPHICS_ON_8BIT);
		drawing = true;
	}
	void LCD12864::endDraw() {
		writeCommand(FOUR_WIRE_INTERFACE ? Command::EXT_GRAPHICS_OFF_4BIT : Command::EXT_GRAPHICS_OFF_8BIT);
		drawing = false;
	}
	
	void LCD12864::clearDrawing() {
        if(!isDrawing()) {
            return;
        }
        
        for(uint8_t row = 0; row < 32; row ++) {
            for(uint8_t col = 0; col < 16; col ++) {
                // The row gets written first
                // There are 32 rows (bottom 32 are just extensions of the top 32)
                // And then the column gets written (16 pixels)
                __NO_INTERRUPT(
                    writeCommand(0x80 | row);
                    writeCommand(0x80 | col);
                    writeData(0x00);
                    writeData(0x00);
                );
                
                // Clear our buffers
                drawBuf[row][col] = 0x0000;
                dispBuf[row][col] = 0x0000;
            }
        }
	}
	// This function takes the drawing buffer, compares it with the display buffer and writes any necessary bytes.
	void LCD12864::updateDrawing() {
        if(!isDrawing()) {
            return;
        }
        for(uint8_t row = 0; row < 32; row ++) {
            for(uint8_t col = 0; col < 16; col ++) {
                // Compare drawBuf with dispBuf
                if(dispBuf[row][col] != drawBuf[row][col]) {
                    // Update the display buffer
                    dispBuf[row][col] = drawBuf[row][col];
                    __NO_INTERRUPT(
                        writeCommand(0x80 | row);
                        writeCommand(0x80 | col);
                        // Write higher order byte first
                        writeData(dispBuf[row][col] >> 8);
                        writeData(dispBuf[row][col] & 0x00FF);
                    );
                }
            }
        }
	}
	
	void LCD12864::clearDrawingBuffer() {
		std::memset(drawBuf, 0, sizeof(drawBuf));
	}
	
	void LCD12864::setPixel(int16_t x, int16_t y, bool state) {
		if(x >= 128 || y >= 64 || x < 0 || y < 0) {
			return;
		}
		
		// Calculate the index of the uint16_t
		uint8_t col = x / 16;
		uint8_t row = y;
		// The LCD has a weird coordinate system; the bottom 32 rows are just extensions of the top 32 rows.
		// So our row is more than 32, subtract 32 and shift the column instead
		if(row >= 32) {
			row -= 32;
			col += 8;
		}
		
		uint8_t offset = x % 16;
		uint16_t mask;
		if(state) {
			mask = 0x8000 >> offset; // 1000 0000 0000 0000
			drawBuf[row][col] |= mask;
		}
		else {
			mask = ~(0x8000 >> offset); // Same as above, only flip the 1s and 0s
			drawBuf[row][col] &= mask;
		}
	}
	
	void LCD12864::ORDrawBufferByte(uint16_t x, uint16_t y, uint8_t data) {
		if(x >= 16 || y >= 64) {
			return;
		}
		// Calculate row and column
		// The row is just the Y if y < 32, otherwise it's y - 32
		uint8_t row = y < 32 ? y : y - 32;
		// The column is just x / 2 if y < 32, otherwise it's x / 2 + 8
		uint8_t col = x / 2 + (y < 32 ? 0 : 8);
		// If x is even, then the byte is on the left of the uint16, so left shift by 8.
		drawBuf[row][col] |= x % 2 == 0 ? data << 8 : data;
	}

	void LCD12864::ANDDrawBufferByte(uint16_t x, uint16_t y, uint8_t data) {
		if(x >= 16 || y >= 64) {
			return;
		}
		// Calculate row and column
		// The row is just the Y if y < 32, otherwise it's y - 32
		uint8_t row = y < 32 ? y : y - 32;
		// The column is just x / 2 if y < 32, otherwise it's x / 2 + 8
		uint8_t col = x / 2 + (y < 32 ? 0 : 8);
		// If x is even, then the byte is on the left of the uint16, so left shift by 8.
		// We also don't want to affect the other byte, so make sure the other byte is all 1s.
		drawBuf[row][col] &= x % 2 == 0 ? data << 8 | 0x00FF : data | 0xFF00;
	}
	
	void LCD12864::drawImage(int16_t x, int16_t y, const Image &Image, bool invert) {
		// Check for out of bounds
		if(x >= 128 || y >= 64) {
			return;
		}
		if(x + Image.width < 0 || y + Image.height < 0) {
			return;
		}

		int16_t baseByte = floorDiv(x, static_cast<int16_t>(8));
		int8_t offset = positiveMod(x, static_cast<int16_t>(8));
		for(int16_t row = 0; row < Image.height; row ++) {
			// If the byte we're drawing into is out of bounds vertically then break this outer loop
			if(row + y >= 64) {
				break;
			}
			// If the byte is too high up, skip this row
			if(row + y < 0) {
				continue;
			}
			for(int16_t byte = 0; byte < Image.bytesWide; byte ++) {
				// If the byte we're drawing into is out of bounds horizontally then break this inner loop
				if(baseByte + byte >= 16) {
					break;
				}
				// If the byte we're drawing into is too far left, skip it
				if(baseByte + byte < 0) {
					continue;
				}
				
				// The bytes have to be shifted
				uint8_t currentByte = Image.data[row * Image.bytesWide + byte] >> offset;
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
					currentByte |= Image.data[row * Image.bytesWide + byte - 1] << (8 - offset);
				}
				
				if(!invert) {
					ORDrawBufferByte(baseByte + byte, row + y, currentByte);
				}
				else {
					ANDDrawBufferByte(baseByte + byte, row + y, ~currentByte);
				}
			}
			// Finally, if we shifted by more than one bit, then there must be some bits clipped in the end
			// Here we recover those lost bits and write them to the buffer
			if(offset != 0) {
				uint8_t finalByte = Image.data[row * Image.bytesWide + Image.bytesWide - 1] << (8 - offset);
				if(!invert) {
					ORDrawBufferByte(baseByte + Image.bytesWide, row + y, finalByte);
				}
				else {
					ANDDrawBufferByte(baseByte + Image.bytesWide, row + y, ~finalByte);
				}
			}
		}
	}
	
	// Bresenham's Line Algorithm
	void drawLineLow(LCD12864 &dest, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
		bool flip = y2 < y1;
		if (flip) {
			y2 = y1 + (y1 - y2);
		}
		dest.setPixel(x1, y1, true);
		int16_t dx = x2 - x1;
		int16_t dy = y2 - y1;
		int16_t dy2 = 2 * dy;
		int16_t dydx2 = dy2 - 2 * dx;

		int16_t p = dy2 - dx;
		int16_t y = y1;
		for (int16_t x = x1 + 1; x <= x2; x++) {
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
	void drawLineHigh(LCD12864 &dest, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
		int16_t temp = x1;
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
		int16_t dx = x2 - x1;
		int16_t dy = y2 - y1;
		int16_t dy2 = 2 * dy;
		int16_t dydx2 = dy2 - 2 * dx;

		int16_t p = dy2 - dx;
		int16_t y = y1;
		for (int16_t x = x1 + 1; x <= x2; x++) {
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

	void LCD12864::drawString(int16_t x, int16_t y, const char *str, bool invert) {
		// Empty string
		if(*str == '\0') {
			return;
		}
		uint16_t width = 0;
		uint16_t height = 0;
		for(uint16_t index = 0; str[index] != '\0'; ++index) {
			const lcd::Image &Image = lcd::getChar(str[index]);
			width += Image.width + 1;
			height = max(height, Image.height);
		}
		// Subtract away one extra spacing
		--width;

		if(invert) {
			// Fill the zone
			fill(x - 1, y - 1, width + 2, height + 2);
		}

		for(; *str != '\0'; ++str) {
			// Out of bounds check #1
			if(x >= 128 || y >= 64) {
				continue;
			}
			const lcd::Image &Image = lcd::getChar(*str);
			// Out of bounds check #2
			if(x + Image.width < 0 || y + Image.height < 0) {
				continue;
			}
			// Make sure everything is bottom-aligned
			drawImage(x, y + (height - Image.height), Image, invert);
			x += Image.width + 1;
		}
	}

	void LCD12864::fill(int16_t x, int16_t y, uint16_t width, uint16_t height, bool invert) {
		// Check for out of bounds
		if(x >= 128 || y >= 64) {
			return;
		}
		if(x + width < 0 || y + height < 0) {
			return;
		}

		int16_t baseByte = floorDiv(x, static_cast<int16_t>(8));
		int8_t offset = positiveMod(x, static_cast<int16_t>(8));
		// Special handling if the area to fill is all in one byte
		if(offset + width < 8) {
			// Check for out of bounds
			if(baseByte < 0) {
				return;
			}
			// Find out what the byte looks like
			uint8_t data = 0xFF >> offset;
			data &= 0xFF << (8 - offset - width);
			for(uint16_t row = 0; row < height; row ++) {
				if(y + row < 0) {
					continue;
				}
				if(invert) {
					ANDDrawBufferByte(baseByte, y + row, ~data);
				}
				else {
					ORDrawBufferByte(baseByte, y + row, data);
				}
			}
			return;
		}
		// Otherwise split into 3 parts
		uint8_t start = 0xFF >> offset;
		// (offset + width) % 8 calculates how many bits are in the last byte
		// Then we shift 0xFF left by 8 minus those bits to get the last byte
		uint8_t end = 0xFF << (8 - (width + offset) % 8);
		uint8_t bytesWide = (offset + width) / 8 - 1;
		for(uint16_t row = 0; row < height; row ++) {
			if(y + row < 0) {
				continue;
			}
			if(invert) {
				ANDDrawBufferByte(baseByte, y + row, ~start);
			}
			else {
				ORDrawBufferByte(baseByte, y + row, start);
			}
			for(uint16_t col = 0; col < bytesWide; col ++) {
				if(invert) {
					ANDDrawBufferByte(baseByte + 1 + col, y + row, 0x00);
				}
				else {
					ORDrawBufferByte(baseByte + 1 + col, y + row, 0xFF);
				}
			}
			if(invert) {
				ANDDrawBufferByte(baseByte + 1 + bytesWide, y + row, ~end);
			}
			else {
				ORDrawBufferByte(baseByte + 1 + bytesWide, y + row, end);
			}
		}
	}

    uint16_t LCD12864::getDrawnStringWidth(const char *str) {
        // Empty string
		if(*str == '\0') {
			return 0;
		}
		uint16_t width = 0;
		for(uint16_t index = 0; str[index] != '\0'; ++index) {
			const lcd::Image &Image = lcd::getChar(str[index]);
			width += Image.width + 1;
		}
		// Subtract away one extra spacing
		--width;
        return width;
    }
	
	#undef writeCommand
	#undef writeData
}
