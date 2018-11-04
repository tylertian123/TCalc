#include "neda.hpp"
#include "lcd12864_charset.hpp"

namespace neda {
	
	uint16_t StringExpression::getWidth() {
		if(contents.length() == 0) {
			return 0;
		}
		
		//Add up all the character's widths
		uint16_t width = 0;
		for(char ch : contents) {
			width += lcd::getChar(ch).width;
		}
		//Add up all length - 1 spaces between the characters
		width += contents.length() - 1;
		return width;
	}
	uint16_t StringExpression::getHeight() {
		uint16_t max = 0;
		//Take the max of all the heights
		for(char ch : contents) {
			max = lcd::getChar(ch).height > max ? lcd::getChar(ch).height : max;
		}
		return max;
	}
	void StringExpression::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		uint16_t height = getHeight();
		for(char ch : contents) {
			if(x >= 128 || y >= 64) {
				return;
			}
			const lcd::Img &charImg = lcd::getChar(ch);
			//To make sure everything is bottom aligned, the max height is added to the y coordinate and the height subtracted from it
			//This is so that characters less than the max height are still displayed properly.
			dest.drawImage(x, y + height - charImg.height, charImg);
			//Increment x and leave one pixel's spacing
			x += charImg.width + 1;
		}
	}
}
