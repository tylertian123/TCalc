#include "neda.hpp"
#include "lcd12864_charset.hpp"

#ifndef MAX
#define MAX(a, b) (b > a ? b : a)
#endif

namespace neda {
	
	//*************************** Expr ***************************************
	uint16_t Expr::getWidth() {
		return exprWidth;
	}
	uint16_t Expr::getHeight() {
		return exprHeight;
	}
	
	//*************************** StringExpr ***************************************
	void StringExpr::computeWidth() {
		if(contents.length() == 0) {
			exprWidth = 0;
			return;
		}
		
		//Add up all the character's widths
		exprWidth = 0;
		for(char ch : contents) {
			exprWidth += lcd::getChar(ch).width;
		}
		//Add up all length - 1 spaces between the characters
		exprWidth += contents.length() - 1;
	}
	void StringExpr::computeHeight() {
		uint16_t max = 0;
		//Take the max of all the heights
		for(char ch : contents) {
			max = lcd::getChar(ch).height > max ? lcd::getChar(ch).height : max;
		}
		exprHeight = max;
	}
	void StringExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
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
	void StringExpr::addChar(char ch) {
		contents.add(ch);
		computeWidth();
		computeHeight();
	}
	
	//*************************** ContainerExpr ***************************************
	void ContainerExpr::computeWidth() {
		//An empty ContainerExpr has a default width and height of 5x9
		if(contents.length() == 0) {
			exprWidth = 5;
			return;
		}
		
		//Add up all the Expressions's widths
		exprWidth = 0;
		for(Expr *ex : contents) {
			exprWidth += ex->getWidth();
		}
		//Add up all length - 1 spaces between the Exprs
		exprWidth += (contents.length() - 1) * 3;
	}
	void ContainerExpr::computeHeight() {
		if(contents.length() == 0) {
			exprHeight = 9;
			return;
		}
		uint16_t max = 0;
		//Take the max of all the heights
		for(Expr *ex : contents) {
			uint16_t height = ex->getHeight();
			max = MAX(height, max);
		}
		exprHeight = max;
	}
	void ContainerExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
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
		
		for(Expr *ex : contents) {
			uint16_t exHeight = ex->getHeight();
			//Center everything
			ex->draw(dest, x, y + (height - exHeight) / 2);
			x += ex->getWidth() + 3;
		}
	}
	void ContainerExpr::addExpr(Expr *expr) {
		contents.add(expr);
		
		computeWidth();
		computeHeight();
	}
	ContainerExpr::~ContainerExpr() {
		for(Expr *ex : contents) {
			delete ex;
		}
	}
	
	//*************************** FractionExpr ***************************************
	void FractionExpr::computeWidth() {
		//Take the greater of the widths and add 2 for the spacing at the sides
		uint16_t numeratorWidth = numerator ? numerator->getWidth() : 0;
		uint16_t denominatorWidth = denominator ? denominator->getWidth() : 0;
		exprWidth = MAX(numeratorWidth, denominatorWidth) + 2;
	}
	void FractionExpr::computeHeight() {
		uint16_t numeratorHeight = numerator ? numerator->getHeight() : 0;
		uint16_t denominatorHeight = denominator ? denominator->getHeight() : 0;
		//Take the sum of the heights and add 3 for the fraction line
		exprHeight = numeratorHeight + denominatorHeight + 3;
	}
	void FractionExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		//Watch out for null pointers
		if(!numerator || !denominator) {
			return;
		}
		
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
	Expr* FractionExpr::getNumerator() {
		return numerator;
	}
	Expr* FractionExpr::getDenominator() {
		return denominator;
	}
	void FractionExpr::setNumerator(Expr *numerator) {
		this->numerator = numerator;
		computeWidth();
		computeHeight();
	}
	void FractionExpr::setDenominator(Expr *denominator) {
		this->denominator = denominator;
		computeWidth();
		computeHeight();
	}
	FractionExpr::~FractionExpr() {
		if(numerator) {
			delete numerator;
		}
		if(denominator) {
			delete denominator;
		}
	}
}
