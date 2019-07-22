#include "lcd12864.hpp"
#include "lcd12864_charset.hpp"
#include "util.hpp"
#include <string.h>

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
            }
        }
        memset(dispBuf, 0, sizeof(dispBuf));
        drawBuf.clear();
	}
	// This function takes the drawing buffer, compares it with the display buffer and writes any necessary bytes.
	void LCD12864::updateDrawing() {
        if(!isDrawing()) {
            return;
        }
        for(uint8_t row = 0; row < 32; row ++) {
            for(uint8_t col = 0; col < 16; col ++) {
                uint16_t data = drawBuf.getLCDWord(row, col);
                // Compare drawBuf with dispBuf
                if(dispBuf[row][col] != data) {
                    // Update the display buffer
                    dispBuf[row][col] = data;
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
		drawBuf.clear();
	}
	
	void LCD12864::setPixel(int16_t x, int16_t y, bool state) {
		drawBuf.setPixel(x, y, state);
	}
	
	void LCD12864::drawImage(int16_t x, int16_t y, const Image &img, bool invert) {
		drawBuf.drawImage(x, y, img, invert);
	}
	
	void LCD12864::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert) {
		drawBuf.drawLine(x1, y1, x2, y2, invert);
	}

	void LCD12864::drawString(int16_t x, int16_t y, const char *str, bool invert) {
		drawBuf.drawString(x, y, str, invert);
	}

	void LCD12864::fill(int16_t x, int16_t y, uint16_t width, uint16_t height, bool invert) {
		drawBuf.fill(x, y, width, height, invert);
	}

    void LCD12864::copyBuffer(const DrawBuf &buf) {
        drawBuf.copy(buf);
    }

    uint16_t LCD12864::getDrawnStringWidth(const char *str) {
        return DrawBuf::getDrawnStringWidth(str);
    }
}
