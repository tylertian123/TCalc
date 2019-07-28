#include "drawbuf.hpp"
#include "lcd12864_charset.hpp"
#include "util.hpp"
#include <string.h>

namespace lcd {

    void DrawBuf::clear() {
        memset(buf, 0, sizeof(buf));
    }

    void DrawBuf::setPixel(int16_t x, int16_t y, bool state) {
        if(x >= 128 || y >= 64 || x < 0 || y < 0) {
			return;
		}
		
        // Calculate the index of the byte
        uint8_t col = x / 8;
        uint8_t row = y;
        
        // The offset of the specific bit in the byte
        uint8_t offset = x % 8;
        // Mask for bit twiddling
        uint8_t mask;

        if(state) {
            mask = 0x80 >> offset;
            buf[row][col] |= mask;
        }
        else {
            mask = ~(0x80 >> offset);
            buf[row][col] &= mask;
        }
    }

    void DrawBuf::drawImage(int16_t x, int16_t y, const Image &img, bool invert) {
        // Check for out of bounds
		if(x >= 128 || y >= 64) {
			return;
		}
		if(x + img.width < 0 || y + img.height < 0) {
			return;
		}

        // Calculate the byte and the offset the image starts in
		int16_t baseByte = util::floorDiv(x, static_cast<int16_t>(8));
		int8_t offset = util::positiveMod(x, static_cast<int16_t>(8));
		for(int16_t row = 0; row < img.height; row ++) {
			// If the byte we're drawing into is out of bounds vertically then break this outer loop
			if(row + y >= 64) {
				break;
			}
			if(row + y < 0) {
				continue;
			}

			for(int16_t byte = 0; byte < img.bytesWide; byte ++) {
				// If the byte we're drawing into is out of bounds horizontally then break this inner loop
				if(baseByte + byte >= 16) {
					break;
				}
				if(baseByte + byte < 0) {
					continue;
				}
				
				// The bytes have to be shifted
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
				
				if(!invert) {
                    buf[row + y][baseByte + byte] |= currentByte;
				}
				else {
                    buf[row + y][baseByte + byte] &= ~currentByte;
				}
			}
			// Finally, if we shifted by more than one bit, then there must be some bits clipped in the end
			// Here we recover those lost bits and write them to the buffer
			if(offset != 0 && baseByte + img.bytesWide < 16) {
				uint8_t finalByte = img.data[row * img.bytesWide + img.bytesWide - 1] << (8 - offset);
				if(!invert) {
                    buf[row + y][baseByte + img.bytesWide] |= finalByte;
				}
				else {
                    buf[row + y][baseByte + img.bytesWide] &= ~finalByte;
				}
			}
		}
    }

    // Bresenham's Line Algorithm
    // Split into two cases: when the slope is 1 or less, and when the slope is greater than 1
	void DrawBuf::drawLineLow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert) {
		bool flip = y2 < y1;
		if (flip) {
			y2 = y1 + (y1 - y2);
		}
		setPixel(x1, y1, !invert);
		int16_t dx = x2 - x1;
		int16_t dy = y2 - y1;
		int16_t dy2 = 2 * dy;
		int16_t dydx2 = dy2 - 2 * dx;

		int16_t p = dy2 - dx;
		int16_t y = y1;
		for (int16_t x = x1 + 1; x <= x2; x++) {
			if (p < 0) {
				setPixel(x, y, !invert);
				p += dy2;
			}
			else {
				setPixel(x, flip ? --y : ++y, !invert);
				p += dydx2;
			}

		}
	}

	void DrawBuf::drawLineHigh(int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert) {
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

		setPixel(y1, x1, !invert);
		int16_t dx = x2 - x1;
		int16_t dy = y2 - y1;
		int16_t dy2 = 2 * dy;
		int16_t dydx2 = dy2 - 2 * dx;

		int16_t p = dy2 - dx;
		int16_t y = y1;
		for (int16_t x = x1 + 1; x <= x2; x++) {
			if (p < 0) {
				setPixel(y, x, !invert);
				p += dy2;
			}
			else {
				setPixel(flip ? y -- : y++, x, !invert);
				p += dydx2;
			}

		}
	}

    void DrawBuf::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool invert) {
		if (util::abs(y2 - y1) <= util::abs(x2 - x1)) {
			if (x2 > x1) {
				drawLineLow(x1, y1, x2, y2, invert);
			}
			else {
				drawLineLow(x2, y2, x1, y1, invert);
			}
		}
		else {
			if (y2 > y1) {
				drawLineHigh(x1, y1, x2, y2, invert);
			}
			else {
				drawLineHigh(x2, y2, x1, y1, invert);
			}
		}
	}

    void DrawBuf::drawString(int16_t x, int16_t y, const char *str, bool invert) {
        // Empty string
		if(*str == '\0') {
			return;
		}
		uint16_t width = 0;
		uint16_t height = 0;
		for(uint16_t index = 0; str[index] != '\0'; ++index) {
			const Image &img = getChar(str[index]);
			width += img.width + 1;
			height = util::max(height, img.height);
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
			const Image &img = getChar(*str);
			// Out of bounds check #2
			if(x + img.width < 0 || y + img.height < 0) {
				continue;
			}
			// Make sure everything is bottom-aligned
			drawImage(x, y + (height - img.height), img, invert);
			x += img.width + 1;
		}
    }

    uint16_t DrawBuf::getDrawnStringWidth(const char *str) {
        // Empty string
		if(*str == '\0') {
			return 0;
		}
		uint16_t width = 0;
		for(uint16_t index = 0; str[index] != '\0'; ++index) {
			const Image &img = getChar(str[index]);
			width += img.width + 1;
		}
		// Subtract away one extra spacing
		--width;
        return width;
    }

    void DrawBuf::fill(int16_t x, int16_t y, uint16_t width, uint16_t height, bool invert) {
        // Check for out of bounds
		if(x >= 128 || y >= 64) {
			return;
		}
		if(x + width < 0 || y + height < 0) {
			return;
		}

		int8_t baseByte = util::floorDiv(x, static_cast<int16_t>(8));
		int8_t offset = util::positiveMod(x, static_cast<int16_t>(8));
		// Special handling if the area to fill is all in one byte
		if(offset + width < 8) {
			// Check for out of bounds
			if(baseByte < 0 || baseByte >= 16) {
				return;
			}
			// Find out what the byte looks like
			uint8_t data = 0xFF >> offset;
			data &= 0xFF << (8 - offset - width);
			for(uint16_t row = 0; row < height; row ++) {
				if(y + row < 0 || y + row >= 64) {
					continue;
				}
				if(invert) {
                    buf[y + row][baseByte] &= ~data;
				}
				else {
					buf[y + row][baseByte] |= data;
				}
			}
		}
        else {
            if(baseByte >= 16) {
                return;
            }
            // Otherwise split into 3 parts, the first byte, the last byte and everything in between
            uint8_t start = 0xFF >> offset;
            // (offset + width) % 8 calculates how many bits are in the last byte
            // Then we shift 0xFF left by 8 minus those bits to get the last byte
            uint8_t end = 0xFF << (8 - (width + offset) % 8);
            // Calculate how many whole bytes there are
            uint8_t bytesWide = (offset + width) / 8 - 1;
            for(uint16_t row = 0; row < height; row ++) {
                if(y + row < 0 || y + row >= 64) {
                    continue;
                }
                // First part
                if(invert) {
                    buf[y + row][baseByte] &= ~start;
                }
                else {
                    buf[y + row][baseByte] |= start;
                }
                // Middle bytes
                for(uint16_t col = 0; col < bytesWide; col ++) {
                    if(baseByte + 1 + col >= 16) {
                        break;
                    }
                    if(invert) {
                        buf[y + row][baseByte + 1 + col] = 0x00;
                    }
                    else {
                        buf[y + row][baseByte + 1 + col] = 0xFF;
                    }
                }
                if(baseByte + 1 + bytesWide < 16) {
                    // Last part
                    if(invert) {
                        buf[y + row][baseByte + 1 + bytesWide] &= ~end;
                    }
                    else {
                        buf[y + row][baseByte + 1 + bytesWide] |= end;
                    }
                }
            }
        }
    }

    uint16_t DrawBuf::getLCDWord(uint8_t row, uint8_t col) {
        // Since the LCD uses 16-bit words as opposed to 8-bit words, each LCD column is twice as wide as a DrawBuf column
        // Therefore multiply the column by 2
        col *= 2;
		// The LCD has a weird coordinate system; the bottom 32 rows are just extensions of the top 32 rows.
		// Therefore if the column is greater than  or equal to16 after the column translation, subtract 16 and increase row instead
        if(col >= 16) {
            col -= 16;
            row += 32;
        }
        // Now the translation is complete, construct the end result
        return buf[row][col] << 8 | buf[row][col + 1];
    }

    void DrawBuf::copy(const DrawBuf &other) {
        for(uint8_t i = 0; i < 64; i ++) {
            for(uint8_t j = 0; j < 16; j ++) {
                buf[i][j] = other.buf[i][j];
            }
        }
    }
}
