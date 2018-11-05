#include "neda.hpp"
#include "lcd12864_charset.hpp"

#ifndef MAX
#define MAX(a, b) (b > a ? b : a)
#endif

namespace neda {
	
	//*************************** StringExpression ***************************************
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
	void StringExpression::addChar(char ch) {
		contents.add(ch);
	}
	
	//*************************** ContainerExpression ***************************************
	uint16_t ContainerExpression::getWidth() {
		//An empty ContainerExpression has a default width and height of 5x9
		if(contents.length() == 0) {
			return 5;
		}
		
		//Add up all the expressions's widths
		uint16_t width = 0;
		for(Expression *ex : contents) {
			width += ex->getWidth();
		}
		//Add up all length - 1 spaces between the expressions
		width += (contents.length() - 1) * 3;
		return width;
	}
	uint16_t ContainerExpression::getHeight() {
		if(contents.length() == 0) {
			return 9;
		}
		uint16_t max = 0;
		//Take the max of all the heights
		for(Expression *ex : contents) {
			uint16_t height = ex->getHeight();
			max = MAX(height, max);
		}
		return max;
	}
	void ContainerExpression::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		if(contents.length() == 0) {
			//Empty container shows up as a box
			for(uint16_t w = 0; w < getWidth(); w ++) {
				dest.setPixel(x + w, y, true);
				dest.setPixel(x + w, y + getHeight(), true);
			}
			for(uint16_t h = 0; h <= getHeight(); h ++) {
				dest.setPixel(x, y + h, true);
				dest.setPixel(x + getWidth(), y + h, true);
			}
		}
		
		uint16_t height = getHeight();
		
		for(Expression *ex : contents) {
			uint16_t exHeight = ex->getHeight();
			//Center everything
			ex->draw(dest, x, y + (height - exHeight) / 2);
			x += ex->getWidth() + 3;
		}
	}
	void ContainerExpression::addExpr(Expression *expr) {
		contents.add(expr);
	}
	
	//*************************** FractionExpression ***************************************
	uint16_t FractionExpression::getWidth() {
		//Take the greater of the widths and add 2 for the spacing at the sides
		return MAX(numerator->getWidth(), denominator->getWidth()) + 2;
	}
	uint16_t FractionExpression::getHeight() {
		//Take the sum of the heights and add 3 for the fraction line
		return numerator->getHeight() + denominator->getHeight() + 3;
	}
	void FractionExpression::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		uint16_t width = getWidth();
		//Center horizontally
		numerator->draw(dest, x + (width - numerator->getWidth()) / 2, y);
		uint16_t numHeight = numerator->getHeight();
		for(uint16_t i = 0; i < width; i ++) {
			//Draw the fraction line
			dest.setPixel(x + i, y + numHeight + 1, true);
		}
		denominator->draw(dest, x + (width - denominator->getWidth()) / 2, y + numHeight + 3);
	}
	Expression* FractionExpression::getNumerator() {
		return numerator;
	}
	Expression* FractionExpression::getDenominator() {
		return denominator;
	}
	void FractionExpression::setNumerator(Expression *numerator) {
		this->numerator = numerator;
	}
	void FractionExpression::setDenominator(Expression *denominator) {
		this->denominator = denominator;
	}
}
