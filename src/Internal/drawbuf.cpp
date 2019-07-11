#include "drawbuf.hpp"
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
        uint8_t offset = x % 16;
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
		int16_t baseByte = floorDiv(x, static_cast<int16_t>(8));
		int8_t offset = positiveMod(x, static_cast<int16_t>(8));
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
                    buf[row + y][baseByte + byte] &= currentByte;
				}
			}
			// Finally, if we shifted by more than one bit, then there must be some bits clipped in the end
			// Here we recover those lost bits and write them to the buffer
			if(offset != 0) {
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

    
}
