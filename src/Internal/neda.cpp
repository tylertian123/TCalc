#include "neda.hpp"
#include "lcd12864_charset.hpp"

#ifndef MAX
#define MAX(a, b) (((b) > (a)) ? (b) : (a))
#endif

//Execute method on obj with no arguments if obj is not null, otherwise 0
#define SAFE_EXEC(obj, method) ((obj) ? (obj->method()) : 0)

namespace neda {
	
	//*************************** Expr ***************************************
	uint16_t Expr::getWidth() {
		return exprWidth;
	}
	uint16_t Expr::getHeight() {
		return exprHeight;
	}
	
	//*************************** StringExpr ***************************************
    uint16_t StringExpr::getTopSpacing() {
        //Because of its basic nature, StringExpr doesn't need any special processing
        return exprHeight / 2;
    }
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
    uint16_t ContainerExpr::getTopSpacing() {
        //ContainerExprs don't contain any special parts; they're just rectangles
        return exprHeight / 2;
    }
	void ContainerExpr::computeWidth() {
		//An empty ContainerExpr has a default width and height
		if(contents.length() == 0) {
			exprWidth = EMPTY_CONTAINER_WIDTH;
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
			exprHeight = EMPTY_CONTAINER_HEIGHT;
			return;
		}

        //Computing the height takes special logic as it is more than just taking the max of all the children's heights.
        //In the case of expressions such as 1^2+3_4, the height is greater than the max of all the children because the 1 and 3
        //have to line up.
		uint16_t maxTopSpacing = 0;
		//Take the max of all the top spacings
		for(Expr *ex : contents) {
			uint16_t ts = ex->getTopSpacing();
			maxTopSpacing = MAX(ts, maxTopSpacing);
		}
        //Now with the max top spacing we can compute the heights and see what the max is
        uint16_t maxHeight = 0;
        for(Expr *ex : contents) {
            //To calculate the height of any expression in this container, we essentially "replace" the top spacing with the max top
            //spacing, so that there is now a padding on the top. This is done in the expression below.
            //When that expression's top spacing is the max top spacing, the expression will be touching the top of the container.
            //Therefore, its height is just the height. In other cases, it will be increased by the difference between the max top
            //spacing and the top spacing.
            uint16_t height = (ex->getHeight() - ex->getTopSpacing()) + maxTopSpacing;
            maxHeight = MAX(height, maxHeight);
        }
        exprHeight = maxHeight;
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
		
        //Special logic in drawing to make sure everything lines up
        uint16_t maxTopSpacing = 0;
        //Take the max of all the top spacings; this will be used to compute the top padding for each expression later.
        for(Expr *ex : contents) {
            uint16_t ts = ex->getTopSpacing();
            maxTopSpacing = MAX(ts, maxTopSpacing);
        }
		
		for(Expr *ex : contents) {
			uint16_t exHeight = ex->getHeight();
			//For each expression, its top padding is the difference between the max top spacing and its top spacing.
            //E.g. A tall expression like 1^2 would have a higher top spacing than 3, so the max top spacing would be its top spacing;
            //So when drawing the 1^2, there is no difference between the max top spacing and the top spacing, and therefore it has
            //no top padding. But when drawing the 3, the difference between its top spacing and the max creates a top padding.
			ex->draw(dest, x, y + (maxTopSpacing - ex->getTopSpacing()));
            //Increase x so nothing overlaps
            //Add 3 for a gap between different expressions
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
    uint16_t FractionExpr::getTopSpacing() {
        //The top spacing of a fraction is equal to the height of its numerator, plus a pixel of spacing between the numerator and
        //the fraction line.
        return SAFE_EXEC(numerator, getHeight) + 1;
    }
	void FractionExpr::computeWidth() {
		//Take the greater of the widths and add 2 for the spacing at the sides
        uint16_t numeratorWidth = SAFE_EXEC(numerator, getWidth);
		uint16_t denominatorWidth = SAFE_EXEC(denominator, getWidth);
		exprWidth = MAX(numeratorWidth, denominatorWidth) + 2;
	}
	void FractionExpr::computeHeight() {
		uint16_t numeratorHeight = SAFE_EXEC(numerator, getHeight);
		uint16_t denominatorHeight = SAFE_EXEC(denominator, getHeight);
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
	
	//*************************** ExponentExpr ***************************************
    uint16_t ExponentExpr::getTopSpacing() {
        //The top spacing for exponents is the height minus half of the height of the base
		uint16_t baseHeight = SAFE_EXEC(base, getHeight);
		//Round down
        return exprHeight - (baseHeight % 2 == 0 ? baseHeight / 2 : baseHeight / 2 + 1);
    }
	void ExponentExpr::computeWidth() {
		uint16_t baseWidth = SAFE_EXEC(base, getWidth);
		uint16_t exponentWidth = SAFE_EXEC(exponent, getWidth);
		exprWidth = baseWidth + exponentWidth + 2;
	}
	void ExponentExpr::computeHeight() {
		uint16_t baseHeight = SAFE_EXEC(base, getHeight);
		uint16_t exponentHeight = SAFE_EXEC(exponent, getHeight);
		//Make sure this is positive
		exprHeight = MAX(0, baseHeight + exponentHeight - BASE_EXPONENT_OVERLAP);
	}
	void ExponentExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		if(!base || !exponent) {
			return;
		}
		uint16_t baseWidth = base->getWidth();
		uint16_t exponentHeight = exponent->getHeight();
		base->draw(dest, x, y + MAX(0, exponentHeight - BASE_EXPONENT_OVERLAP));
		exponent->draw(dest, x + baseWidth + 2, y);
	}
	Expr* ExponentExpr::getBase() {
		return base;
	}
	Expr* ExponentExpr::getExponent() {
		return exponent;
	}
	void ExponentExpr::setBase(Expr *base) {
		this->base = base;
		computeWidth();
		computeHeight();
	}
	void ExponentExpr::setExponent(Expr *exponent) {
		this->exponent = exponent;
		computeWidth();
		computeHeight();
	}
	ExponentExpr::~ExponentExpr() {
		if(base) {
			delete base;
		}
		if(exponent) {
			delete exponent;
		}
	}
	
	//*************************** BracketExpr ***************************************
	uint16_t BracketExpr::getTopSpacing() {
		//Return the top spacing of the stuff inside, instead of half the height so that things in and out the brackets line up nicely
		return SAFE_EXEC(contents, getTopSpacing) + 1;
	}
	void BracketExpr::computeWidth() {
		//+6 for the brackets themselves and +2 for the spacing
		exprWidth = SAFE_EXEC(contents, getWidth) + 8;
	}
	void BracketExpr::computeHeight() {
		//+2 for the padding at the top and bottom
		exprHeight = SAFE_EXEC(contents, getHeight) + 2;
	}
	void BracketExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		if(!contents) {
			return;
		}
		//Bracket
		dest.setPixel(x + 2, y, true);
		dest.setPixel(x + 1, y + 1, true);
		dest.setPixel(x + 1, y + exprHeight - 1 - 1, true);
		dest.setPixel(x + 2, y + exprHeight - 1, true);
		for(uint16_t i = 2; i < exprHeight - 2; i ++) {
			dest.setPixel(x, y + i, true);
			dest.setPixel(x + exprWidth - 1, y + i, true);
		}
		dest.setPixel(x + exprWidth - 1 - 2, y, true);
		dest.setPixel(x + exprWidth - 1 - 1, y + 1, true);
		dest.setPixel(x + exprWidth - 1 - 1, y + exprHeight - 1 - 1, true);
		dest.setPixel(x + exprWidth - 1 - 2, y + exprHeight - 1, true);
		contents->draw(dest, x + 4, y + 1);
	}
	
	//*************************** RadicalExpr ***************************************
	uint16_t RadicalExpr::getTopSpacing() {
		if(!n) {
			return SAFE_EXEC(contents, getTopSpacing) + 2;
		}
		uint16_t regularTopSpacing = SAFE_EXEC(contents, getTopSpacing) + 2;
		return regularTopSpacing + MAX(0, n->getHeight() - CONTENTS_N_OVERLAP);
	}
	void RadicalExpr::computeWidth() {
		if(!n) {
			exprWidth = SAFE_EXEC(contents, getWidth) + 8;
			return;
		}
		exprWidth = MAX(0, n->getWidth() - SIGN_N_OVERLAP) + SAFE_EXEC(contents, getWidth) + 8;
	}
	void RadicalExpr::computeHeight() {
		if(!n) {
			exprHeight = SAFE_EXEC(contents, getHeight) + 2;
			return;
		}
		exprHeight = MAX(0, n->getHeight() - CONTENTS_N_OVERLAP) + SAFE_EXEC(contents, getHeight) + 2;
	}
	void RadicalExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		if(!n) {
			if(!contents) {
				return;
			}
			dest.drawLine(x, y + exprHeight - 1 - 2, x + 2, y + exprHeight - 1);
			dest.drawLine(x + 2, y + exprHeight - 1, x + 6, 0);
			dest.drawLine(x + 6, 0, x + exprWidth - 1, 0);
			
			contents->draw(dest, x + 7, y + 2);
		}
		else {
			n->draw(dest, 0, 0);
			uint16_t xoffset = MAX(0, n->getWidth() - SIGN_N_OVERLAP);
			uint16_t yoffset = MAX(0, n->getHeight() - CONTENTS_N_OVERLAP);
			dest.drawLine(x + xoffset, y + exprHeight - 1 - 2, x + 2 + xoffset, y + exprHeight - 1);
			dest.drawLine(x + 2 + xoffset, y + exprHeight - 1, x + 6 + xoffset, 0 + yoffset);
			dest.drawLine(x + 6 + xoffset, 0 + yoffset, x + exprWidth - 1 + xoffset, 0 + yoffset);
			
			contents->draw(dest, x + 7 + xoffset, y + 2 + yoffset);
		}
	}
	Expr* RadicalExpr::getContents() {
		return contents;
	}
	Expr* RadicalExpr::getN() {
		return n;
	}
	void RadicalExpr::setContents(Expr *contents) {
		this->contents = contents;
		computeWidth();
		computeHeight();
	}
	void RadicalExpr::setN(Expr *n) {
		this->n = n;
		computeWidth();
		computeHeight();
	}
	
	//*************************** SubscriptExpr ***************************************
	uint16_t SubscriptExpr::getTopSpacing() {
		return SAFE_EXEC(contents, getHeight) / 2;
	}
	void SubscriptExpr::computeWidth() {
		if (subscript) {
			exprWidth = SAFE_EXEC(contents, getWidth) + subscript->getWidth() + 2;
		}
		else {
			exprWidth = SAFE_EXEC(contents, getWidth);
		}
	}
	void SubscriptExpr::computeHeight() {
		if (subscript) {
			exprHeight = SAFE_EXEC(contents, getHeight) + SAFE_EXEC(subscript, getHeight) - CONTENTS_SUBSCRIPT_OVERLAP;
		}
		else {
			exprHeight = SAFE_EXEC(contents, getHeight);
		}
	}
	void SubscriptExpr::draw(lcd::LCD12864 &dest, uint16_t x, uint16_t y) {
		if (!contents) {
			return;
		}
		contents->draw(dest, x, y);
		if (!subscript) {
			return;
		}
		subscript->draw(dest, x + contents->getWidth() + 2, y + contents->getHeight() - CONTENTS_SUBSCRIPT_OVERLAP);
	}
	Expr* SubscriptExpr::getContents() {
		return contents;
	}
	Expr* SubscriptExpr::getSubscript() {
		return subscript;
	}
	void SubscriptExpr::setContents(Expr *contents) {
		this->contents = contents;
		computeWidth();
		computeHeight();
	}
	void SubscriptExpr::setSubscript(Expr *subscript) {
		this->subscript = subscript;
		computeWidth();
		computeHeight();
	}
}

#undef SAFE_EXEC
