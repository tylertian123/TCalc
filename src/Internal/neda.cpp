#include "neda.hpp"
#include "util.hpp"
#include "lcd12864_charset.hpp"

//Execute method on obj with arguments if obj is not null, otherwise 0
#define SAFE_EXEC(obj, method, ...) if(obj) (obj)->method(__VA_ARGS__)
#define SAFE_EXEC_0(obj, method, ...) ((obj) ? (obj->method(__VA_ARGS__)) : 0)
#define VERIFY_INBOUNDS(x, y) if(x >= 128 || y >= 64 || x + exprWidth < 0 || y + exprHeight < 0) return
#define ASSERT_NONNULL(obj) if(!obj) return
#define DESTROY_IF_NONNULL(obj) if(obj) delete obj

namespace neda {
	
	//*************************** Expr ***************************************
	uint16_t Expr::getWidth() {
		return exprWidth;
	}
	uint16_t Expr::getHeight() {
		return exprHeight;
	}
    void Expr::draw(lcd::LCD12864 &dest) {
        draw(dest, x, y);
    }
    int16_t Expr::getX() {
        return x;
    }
    int16_t Expr::getY() {
        return y;
    }
    //Default impl: Call the parent's cursor method, if it has one
    void Expr::left(Expr *ex, Cursor &cursor) {
        SAFE_EXEC(parent, left, this, cursor);
    }
    void Expr::right(Expr *ex, Cursor &cursor) {
        SAFE_EXEC(parent, right, this, cursor);
    }
    void Expr::up(Expr *ex, Cursor &cursor) {
        SAFE_EXEC(parent, up, this, cursor);
    }
    void Expr::down(Expr *ex, Cursor &cursor) {
        SAFE_EXEC(parent,down, this, cursor);
    }
    void Expr::drawConnected(lcd::LCD12864 &dest) {
        if(parent) {
            parent->drawConnected(dest);
        }
        else {
            draw(dest);
        }
    }
	
	//*************************** StringExpr ***************************************
    uint16_t StringExpr::getTopSpacing() {
        //Because of its basic nature, StringExpr doesn't need any special processing
        return exprHeight / 2;
    }
	void StringExpr::computeWidth() {
		if(contents->length() == 0) {
			exprWidth = 0;
            SAFE_EXEC(parent, computeWidth);
			return;
		}
		
		//Add up all the character's widths
		exprWidth = 0;
		for(char ch : *contents) {
			exprWidth += lcd::getChar(ch).width;
		}
		//Add up all length - 1 spaces between the characters
		exprWidth += contents->length() - 1;
        SAFE_EXEC(parent, computeWidth);
	}
	void StringExpr::computeHeight() {
        if(contents->length() == 0) {
            exprHeight = ContainerExpr::EMPTY_EXPR_HEIGHT;
            SAFE_EXEC(parent, computeHeight);
            return;
        }
		uint16_t max = 0;
		//Take the max of all the heights
		for(char ch : *contents) {
			max = lcd::getChar(ch).height > max ? lcd::getChar(ch).height : max;
		}
		exprHeight = max;
        SAFE_EXEC(parent, computeHeight);
	}
	void StringExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);

        if(contents->length() == 0) {
            return;
        }

		for(char ch : *contents) {
			if(x >= 128 || y >= 64) {
				return;
			}
            
			const lcd::Img &charImg = lcd::getChar(ch);
            //Skip this char and increment if it's outside the range
            if(x + charImg.width < 0 || y + charImg.height < 0) {
                //Increment x and leave one pixel's spacing
                x += charImg.width + 1;
                continue;
            }
			//To make sure everything is bottom aligned, the max height is added to the y coordinate and the height subtracted from it
			//This is so that characters less than the max height are still displayed properly.
			dest.drawImage(x, y + exprHeight - charImg.height, charImg);
			//Increment x and leave one pixel's spacing
			x += charImg.width + 1;
		}
	}
	void StringExpr::addChar(char ch) {
		contents->add(ch);
		computeWidth();
		computeHeight();
	}
    void StringExpr::addAtCursor(char ch, Cursor &cursor) {
        contents->insert(ch, cursor.index);
		cursor.index ++;
		computeWidth();
		computeHeight();
    }
    void StringExpr::removeAtCursor(Cursor &cursor) {
        if(cursor.index != 0) {
            contents->removeAt(--cursor.index);
        }
		computeWidth();
		computeHeight();
    }
    void StringExpr::drawCursor(lcd::LCD12864 &dest, const Cursor &cursor) {
        int16_t cursorX = x;
        for(uint16_t i = 0; i < cursor.index && i < contents->length(); i ++) {
            cursorX += lcd::getChar((*contents)[i]).width;
        }
        if(cursor.index != 0) {
            cursorX += cursor.index - 1;
        }
        for(int16_t i = 0; i < exprHeight; i ++) {
            dest.setPixel(cursorX, y + i, true);
            dest.setPixel(cursorX + 1, y + i, true);
        }
    }
    StringExpr* StringExpr::beforeCursor(const Cursor &cursor) {
        DynamicArray<char> *a = new DynamicArray<char>(contents->begin(), contents->begin() + cursor.index);
        return new StringExpr(a);
    }
    StringExpr* StringExpr::afterCursor(const Cursor &cursor) {
        DynamicArray<char> *b = new DynamicArray<char>(contents->begin() + cursor.index + 1, contents->end());
        return new StringExpr(b);
    }
    void StringExpr::getCursorInfo(const Cursor &cursor, CursorInfo &out) {
        int16_t cursorX = x;
        for(uint16_t i = 0; i < cursor.index && i < contents->length(); i ++) {
            cursorX += lcd::getChar((*contents)[i]).width;
        }
        if(cursor.index != 0) {
            cursorX += cursor.index - 1;
        }
        out.x = cursorX;
        out.y = y;
        out.width = 2;
        out.height = exprHeight;
    }
    StringExpr::~StringExpr() {
        delete contents;
    }
    void StringExpr::left(Expr *ex, Cursor &cursor) {
        if(cursor.index == 0) {
            SAFE_EXEC(parent, left, this, cursor);
        }
        else {
            cursor.index --;
        }
    }
    void StringExpr::right(Expr *ex, Cursor &cursor) {
        if(cursor.index == contents->length()) {
            SAFE_EXEC(parent, right, this, cursor);
        }
        else {
            cursor.index ++;
        }
    }
    void StringExpr::getCursor(Cursor &cursor, CursorLocation location) {
        cursor.expr = this;
        cursor.index = location == CURSORLOCATION_START ? 0 : contents->length();
    }
    void StringExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
    }
	
	//*************************** ContainerExpr ***************************************
    uint16_t ContainerExpr::getTopSpacing() {
        //ContainerExprs don't contain any special parts; they're just rectangles
        return exprHeight / 2;
    }
	void ContainerExpr::computeWidth() {
		//An empty ContainerExpr has a default width and height
        //A ContainerExpr with only an empty StringExpr inside also has a default width and height
		if(contents.length() == 0
                || (contents.length() == 1 && contents[0]->getType() == ExprType::STRING
                        && ((StringExpr*) contents[0])->contents->length() == 0)) {
			exprWidth = EMPTY_EXPR_WIDTH;
            SAFE_EXEC(parent, computeWidth);
            return;
		}
		
		//Add up all the Expressions's widths
		exprWidth = 0;
		for(Expr *ex : contents) {
			exprWidth += SAFE_EXEC_0(ex, getWidth);
		}
		//Add up all length - 1 spaces between the Exprs
		exprWidth += (contents.length() - 1) * 3;
        SAFE_EXEC(parent, computeWidth);
	}
	void ContainerExpr::computeHeight() {
		if(contents.length() == 0
                || (contents.length() == 1 && contents[0]->getType() == ExprType::STRING
                        && ((StringExpr*) contents[0])->contents->length() == 0)) {
			exprHeight = EMPTY_EXPR_HEIGHT;
            SAFE_EXEC(parent, computeHeight);
			return;
		}

        //Computing the height takes special logic as it is more than just taking the max of all the children's heights.
        //In the case of expressions such as 1^2+3_4, the height is greater than the max of all the children because the 1 and 3
        //have to line up.
		uint16_t maxTopSpacing = 0;
		//Take the max of all the top spacings
		for(Expr *ex : contents) {
			uint16_t ts = SAFE_EXEC_0(ex, getTopSpacing);
			maxTopSpacing = max(ts, maxTopSpacing);
		}
        //Now with the max top spacing we can compute the heights and see what the max is
        uint16_t maxHeight = 0;
        for(Expr *ex : contents) {
            //To calculate the height of any expression in this container, we essentially "replace" the top spacing with the max top
            //spacing, so that there is now a padding on the top. This is done in the expression below.
            //When that expression's top spacing is the max top spacing, the expression will be touching the top of the container.
            //Therefore, its height is just the height. In other cases, it will be increased by the difference between the max top
            //spacing and the top spacing.
            uint16_t height = (SAFE_EXEC_0(ex, getHeight) - SAFE_EXEC_0(ex, getTopSpacing)) + maxTopSpacing;
            maxHeight = max(height, maxHeight);
        }
        exprHeight = maxHeight;
        SAFE_EXEC(parent, computeHeight);
	}
	void ContainerExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);

        if(contents.length() == 0
            || (contents.length() == 1 && contents[0]->getType() == ExprType::STRING
                    && ((StringExpr*) contents[0])->contents->length() == 0)) {
            //Empty container shows up as a box
            for(uint16_t w = 0; w < exprWidth; w ++) {
                dest.setPixel(x + w, y, true);
                dest.setPixel(x + w, y + exprHeight - 1, true);
            }
            for(uint16_t h = 0; h < exprHeight; h ++) {
                dest.setPixel(x, y + h, true);
                dest.setPixel(x + exprWidth - 1, y + h, true);
            }
            return;
        }
		
        //Special logic in drawing to make sure everything lines up
        uint16_t maxTopSpacing = 0;
        //Take the max of all the top spacings; this will be used to compute the top padding for each expression later.
        for(Expr *ex : contents) {
            uint16_t ts = SAFE_EXEC_0(ex, getTopSpacing);
            maxTopSpacing = max(ts, maxTopSpacing);
        }
		
		for(Expr *ex : contents) {
            if(!ex) {
                continue;
            }
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
		expr->parent = this;
		contents.add(expr);
		
		computeWidth();
		computeHeight();
	}
    void ContainerExpr::removeExpr(Expr *expr) {
        for(uint16_t i = 0; i < contents.length(); i ++) {
            if(contents[i] == expr) {
                contents.removeAt(i);
                break;
            }
        }
    }
    void ContainerExpr::replaceExpr(Expr *exprToReplace, Expr *replacement) {
        for(auto it = contents.begin(); it != contents.end(); it ++) {
            if(*it == exprToReplace) {
                *it = replacement;
                replacement->parent = this;
                computeWidth();
                computeHeight();
                break;
            }
        }
    }
    void ContainerExpr::addAfter(Expr *expr, Expr *exprToAdd) {
        for(uint16_t i = 0; i < contents.length(); i ++) {
            if(contents[i] == expr) {
                contents.insert(exprToAdd, i + 1);
                exprToAdd->parent = this;
                computeWidth();
                computeHeight();
                break;
            }
        }
    }
    
	ContainerExpr::~ContainerExpr() {
		for(Expr *ex : contents) {
			DESTROY_IF_NONNULL(ex);
		}
	}
    void ContainerExpr::left(Expr *ex, Cursor &cursor) {
        //First check if the cursor is already in the leftmost element
        if(contents.length() > 0 && ex == contents[0]) {
            SAFE_EXEC(parent, left, this, cursor);
        }
        else {
            //Compare ex to all elements
            for(uint16_t i = 1; i < contents.length(); i ++) {
                if(contents[i] == ex) {
                    //Move the cursor to the end of the element before
                    contents[i - 1]->getCursor(cursor, CURSORLOCATION_END);
                }
            }
        }
    }
    void ContainerExpr::right(Expr *ex, Cursor &cursor) {
        if(contents.length() > 0 && ex == contents[contents.length() - 1]) {
            SAFE_EXEC(parent, right, this, cursor);
            return;
        }
        else {
            for(uint16_t i = 0; i < contents.length() - 1; i ++) {
                if(contents[i] == ex) {
                    contents[i + 1]->getCursor(cursor, CURSORLOCATION_START);
                }
            }
        }
    }
    void ContainerExpr::getCursor(Cursor &cursor, CursorLocation location) {
        if(contents.length() == 0) {
            return;
        }
        if(location == CURSORLOCATION_START) {
            contents[0]->getCursor(cursor, location);
        }
        else {
            contents[contents.length() - 1]->getCursor(cursor, location);
        }
    }
    void ContainerExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        for(Expr *ex : contents) {
            SAFE_EXEC(ex, updatePosition, dx, dy);
        }
    }
	
	//*************************** FractionExpr ***************************************
    uint16_t FractionExpr::getTopSpacing() {
        //The top spacing of a fraction is equal to the height of its numerator, plus a pixel of spacing between the numerator and
        //the fraction line.
        return SAFE_EXEC_0(numerator, getHeight) + 1;
    }
	void FractionExpr::computeWidth() {
		//Take the greater of the widths and add 2 for the spacing at the sides
        uint16_t numeratorWidth = SAFE_EXEC_0(numerator, getWidth);
		uint16_t denominatorWidth = SAFE_EXEC_0(denominator, getWidth);
		exprWidth = max(numeratorWidth, denominatorWidth) + 2;
        SAFE_EXEC(parent, computeWidth);
	}
	void FractionExpr::computeHeight() {
		uint16_t numeratorHeight = SAFE_EXEC_0(numerator, getHeight);
		uint16_t denominatorHeight = SAFE_EXEC_0(denominator, getHeight);
		//Take the sum of the heights and add 3 for the fraction line
		exprHeight = numeratorHeight + denominatorHeight + 3;
        SAFE_EXEC(parent, computeHeight);
	}
	void FractionExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		//Watch out for null pointers
        ASSERT_NONNULL(numerator);
        ASSERT_NONNULL(denominator);
		
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
        numerator->parent = this;
		computeWidth();
		computeHeight();
	}
	void FractionExpr::setDenominator(Expr *denominator) {
		this->denominator = denominator;
        denominator->parent = this;
		computeWidth();
		computeHeight();
	}
	FractionExpr::~FractionExpr() {
        DESTROY_IF_NONNULL(numerator);
        DESTROY_IF_NONNULL(denominator);
	}
    void FractionExpr::up(Expr *ex, Cursor &cursor) {
        if(ex == denominator) {
            SAFE_EXEC(numerator, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, up, this, cursor);
        }
    }
    void FractionExpr::down(Expr *ex, Cursor &cursor) {
        if(ex == numerator) {
            SAFE_EXEC(denominator, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, down, this, cursor);
        }
    }
    void FractionExpr::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            SAFE_EXEC(numerator, getCursor, cursor, location);
        }
        else {
            SAFE_EXEC(denominator, getCursor, cursor, location);
        }
    }
    void FractionExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(numerator, updatePosition, dx, dy);
        SAFE_EXEC(denominator, updatePosition, dx, dy);
    }
	
	//*************************** ExponentExpr ***************************************
    uint16_t ExponentExpr::getTopSpacing() {
        //The top spacing for exponents is the height minus half of the height of the base
		uint16_t baseHeight = SAFE_EXEC_0(base, getHeight);
		//Round down
        return exprHeight - (baseHeight % 2 == 0 ? baseHeight / 2 : baseHeight / 2 + 1);
    }
	void ExponentExpr::computeWidth() {
		uint16_t baseWidth = SAFE_EXEC_0(base, getWidth);
		uint16_t exponentWidth = SAFE_EXEC_0(exponent, getWidth);
		exprWidth = baseWidth + exponentWidth + 2;
        SAFE_EXEC(parent, computeWidth);
	}
	void ExponentExpr::computeHeight() {
		uint16_t baseHeight = SAFE_EXEC_0(base, getHeight);
		uint16_t exponentHeight = SAFE_EXEC_0(exponent, getHeight);
		//Make sure this is positive
		exprHeight = max(0, baseHeight + exponentHeight - BASE_EXPONENT_OVERLAP);
        SAFE_EXEC(parent, computeHeight);
	}
	void ExponentExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		ASSERT_NONNULL(base);
        ASSERT_NONNULL(exponent);
		uint16_t baseWidth = base->getWidth();
		uint16_t exponentHeight = exponent->getHeight();
		base->draw(dest, x, y + max(0, exponentHeight - BASE_EXPONENT_OVERLAP));
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
        base->parent = this;
		computeWidth();
		computeHeight();
	}
	void ExponentExpr::setExponent(Expr *exponent) {
		this->exponent = exponent;
        exponent->parent = this;
		computeWidth();
		computeHeight();
	}
	ExponentExpr::~ExponentExpr() {
        DESTROY_IF_NONNULL(base);
        DESTROY_IF_NONNULL(exponent);
	}
    void ExponentExpr::left(Expr *ex, Cursor &cursor) {
        if(ex == exponent) {
            SAFE_EXEC(base, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, left, this, cursor);
        }
    }
    void ExponentExpr::right(Expr *ex, Cursor &cursor) {
        if(ex == base) {
            SAFE_EXEC(exponent, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void ExponentExpr::up(Expr *ex, Cursor &cursor) {
        if(ex == base) {
            SAFE_EXEC(exponent, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, up, this, cursor);
        }
    }
    void ExponentExpr::down(Expr *ex, Cursor &cursor) {
        if(ex == exponent) {
            SAFE_EXEC(base, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, down, this, cursor);
        }
    }
    void ExponentExpr::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            SAFE_EXEC(base, getCursor, cursor, location);
        }
        else {
            SAFE_EXEC(exponent, getCursor, cursor, location);
        }
    }
    void ExponentExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(base, updatePosition, dx, dy);
        SAFE_EXEC(exponent, updatePosition, dx, dy);
    }
	
	//*************************** BracketExpr ***************************************
	uint16_t BracketExpr::getTopSpacing() {
		//Return the top spacing of the stuff inside, instead of half the height so that things in and out the brackets line up nicely
		return SAFE_EXEC_0(contents, getTopSpacing) + 1;
	}
	void BracketExpr::computeWidth() {
		//+6 for the brackets themselves and +2 for the spacing
		exprWidth = SAFE_EXEC_0(contents, getWidth) + 8;
        SAFE_EXEC(parent, computeWidth);
	}
	void BracketExpr::computeHeight() {
		//+2 for the padding at the top and bottom
		exprHeight = SAFE_EXEC_0(contents, getHeight) + 2;
        SAFE_EXEC(parent, computeHeight);
	}
	void BracketExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		ASSERT_NONNULL(contents);
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
	BracketExpr::~BracketExpr() {
		DESTROY_IF_NONNULL(contents);
	}
    void BracketExpr::getCursor(Cursor &cursor, CursorLocation location) {
        SAFE_EXEC(contents, getCursor, cursor, location);
    }
    void BracketExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
    }
	
	//*************************** RadicalExpr ***************************************
	uint16_t RadicalExpr::getTopSpacing() {
		if(!n) { 
			return SAFE_EXEC_0(contents, getTopSpacing) + 2;
		}
		uint16_t regularTopSpacing = SAFE_EXEC_0(contents, getTopSpacing) + 2;
		return regularTopSpacing + max(0, n->getHeight() - CONTENTS_N_OVERLAP);
	}
	void RadicalExpr::computeWidth() {
		if(!n) {
			exprWidth = SAFE_EXEC_0(contents, getWidth) + 8;
            SAFE_EXEC(parent, computeWidth);
			return;
		}
		exprWidth = max(0, n->getWidth() - SIGN_N_OVERLAP) + SAFE_EXEC_0(contents, getWidth) + 8;
        SAFE_EXEC(parent, computeWidth);
	}
	void RadicalExpr::computeHeight() {
		if(!n) {
			exprHeight = SAFE_EXEC_0(contents, getHeight) + 2;
            SAFE_EXEC(parent, computeHeight);
			return;
		}
		exprHeight = max(0, n->getHeight() - CONTENTS_N_OVERLAP) + SAFE_EXEC_0(contents, getHeight) + 2;
        SAFE_EXEC(parent, computeHeight);
	}
	void RadicalExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		if(!n) {
			ASSERT_NONNULL(contents);
			dest.drawLine(x, y + exprHeight - 1 - 2, x + 2, y + exprHeight - 1);
			dest.drawLine(x + 2, y + exprHeight - 1, x + 6, 0);
			dest.drawLine(x + 6, 0, x + exprWidth - 1, 0);
			
			contents->draw(dest, x + 7, y + 2);
		}
		else {
			n->draw(dest, 0, 0);
			uint16_t xoffset = max(0, n->getWidth() - SIGN_N_OVERLAP);
			uint16_t yoffset = max(0, n->getHeight() - CONTENTS_N_OVERLAP);
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
        contents->parent = this;
		computeWidth();
		computeHeight();
	}
	void RadicalExpr::setN(Expr *n) {
		this->n = n;
        n->parent = this;
		computeWidth();
		computeHeight();
	}
	RadicalExpr::~RadicalExpr() {
        DESTROY_IF_NONNULL(contents);
        DESTROY_IF_NONNULL(n);
	}
    void RadicalExpr::left(Expr *ex, Cursor &cursor) {
        //Move cursor to n only if the cursor is currently under the radical sign, and n is non-null
        if(ex == contents && n) {
            SAFE_EXEC(n, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, left, this, cursor);
        }
    }
    void RadicalExpr::right(Expr *ex, Cursor &cursor) {
        if(ex == n) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void RadicalExpr::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            if(n) {
                n->getCursor(cursor, location);
            }
            else {
                SAFE_EXEC(contents, getCursor, cursor, location);
            }
        }
        else {
            SAFE_EXEC(contents, getCursor, cursor, location);
        }
    }
    void RadicalExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
        SAFE_EXEC(n, updatePosition, dx, dy);
    }
	
	//*************************** SubscriptExpr ***************************************
	uint16_t SubscriptExpr::getTopSpacing() {
		return SAFE_EXEC_0(contents, getHeight) / 2;
	}
	void SubscriptExpr::computeWidth() {
		if (subscript) {
			exprWidth = SAFE_EXEC_0(contents, getWidth) + subscript->getWidth() + 2;
		}
		else {
			exprWidth = SAFE_EXEC_0(contents, getWidth);
		}
        SAFE_EXEC(parent, computeWidth);
	}
	void SubscriptExpr::computeHeight() {
		if (subscript) {
			exprHeight = SAFE_EXEC_0(contents, getHeight) + SAFE_EXEC_0(subscript, getHeight) - CONTENTS_SUBSCRIPT_OVERLAP;
		}
		else {
			exprHeight = SAFE_EXEC_0(contents, getHeight);
		}
        SAFE_EXEC(parent, computeHeight);
	}
	void SubscriptExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
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
        contents->parent = this;
		computeWidth();
		computeHeight();
	}
	void SubscriptExpr::setSubscript(Expr *subscript) {
		this->subscript = subscript;
        subscript->parent = this;
		computeWidth();
		computeHeight();
	}
	SubscriptExpr::~SubscriptExpr() {
        DESTROY_IF_NONNULL(subscript);
        DESTROY_IF_NONNULL(contents);
	}
    void SubscriptExpr::left(Expr *ex, Cursor &cursor) {
        if(ex == subscript) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, left, this, cursor);
        }
    }
    void SubscriptExpr::right(Expr *ex, Cursor &cursor) {
        if(ex == contents) {
            if(subscript) {
                subscript->getCursor(cursor, CURSORLOCATION_START);
            }
            else {
                SAFE_EXEC(parent, right, this, cursor);
            }
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void SubscriptExpr::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            SAFE_EXEC(contents, getCursor, cursor, location);
        }
        else {
            if(subscript) {
                subscript->getCursor(cursor, location);
            }
            else {
                SAFE_EXEC(contents, getCursor, cursor, location);
            }
        }
    }
    void SubscriptExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
        SAFE_EXEC(subscript, updatePosition, dx, dy);
    }
	
	//*************************** SigmaPiExpr ***************************************
	uint16_t SigmaPiExpr::getTopSpacing() {
        //The top spacing of this expr can be split into two cases: when the contents are tall and when the contents are short.
        //When the contents are tall enough, the result is simply the top spacing of the contents (b)
        //Otherwise, it is the distance from the top to the middle of the base of the contents.
        //Therefore, we add up the heights of the expr at the top, the spacing, and the overlap between the symbol and the contents,
        //then subtract half of the base height of the contents (height - top spacing)
		uint16_t a = SAFE_EXEC_0(finish, getHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_EXEC_0(contents, getHeight) - SAFE_EXEC_0(contents, getTopSpacing));
		uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
		return max(a, b);
	}
	void SigmaPiExpr::computeWidth() {
		uint16_t topWidth = SAFE_EXEC_0(finish, getWidth);
		uint16_t bottomWidth = SAFE_EXEC_0(start, getWidth);
		exprWidth = max(symbol.width, max(topWidth, bottomWidth)) + 3 + SAFE_EXEC_0(contents, getWidth);
        SAFE_EXEC(parent, computeWidth);
	}
	void SigmaPiExpr::computeHeight() {
        //Top spacings - Taken from neda::SigmaPiExpr::getTopSpacing()
        uint16_t a = SAFE_EXEC_0(finish, getHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_EXEC_0(contents, getHeight) - SAFE_EXEC_0(contents, getTopSpacing));
        uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
        uint16_t maxTopSpacing = max(a, b);
        //Logic same as neda::ContainerExpr::getHeight()
        uint16_t symbolHeight = SAFE_EXEC_0(finish, getHeight) + 2 + symbol.height + 2 + SAFE_EXEC_0(start, getHeight)
                + maxTopSpacing - a;
        uint16_t bodyHeight = SAFE_EXEC_0(contents, getHeight) + maxTopSpacing - b;

		exprHeight = max(symbolHeight, bodyHeight);
        SAFE_EXEC(parent, computeHeight);
	}
	void SigmaPiExpr::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		ASSERT_NONNULL(start);
        ASSERT_NONNULL(finish);
        ASSERT_NONNULL(contents);
        
        //Top spacings - Taken from neda::SigmaPiExpr::getTopSpacing()
        uint16_t a = SAFE_EXEC_0(finish, getHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_EXEC_0(contents, getHeight) - SAFE_EXEC_0(contents, getTopSpacing));
        uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
        uint16_t maxTopSpacing = max(a, b);
        //Logic same as neda::ContainerExpr::draw()
        uint16_t symbolYOffset = maxTopSpacing - a;
        uint16_t contentsYOffset = maxTopSpacing - b;

        //Center the top, the bottom and the symbol
        uint16_t widest = max(start->getWidth(), max(finish->getWidth(), symbol.width));
        finish->draw(dest, x + (widest - finish->getWidth()) / 2, y + symbolYOffset);
        dest.drawImage(x + (widest - symbol.width) / 2, y + finish->getHeight() + 2 + symbolYOffset, symbol);
        start->draw(dest, x + (widest - start->getWidth()) / 2, y + finish->getHeight() + 2 + symbol.height + 2 + symbolYOffset);

        contents->draw(dest, widest + 3, y + contentsYOffset);
	}
    Expr* SigmaPiExpr::getStart() {
        return start;
    }
    Expr* SigmaPiExpr::getFinish() {
        return finish;
    }
    Expr* SigmaPiExpr::getContents() {
        return contents;
    }
    void SigmaPiExpr::setStart(Expr *start) {
        this->start = start;
        start->parent = this;
        computeWidth();
        computeHeight();
    }
    void SigmaPiExpr::setFinish(Expr *finish) {
        this->finish = finish;
        finish->parent = this;
        computeWidth();
        computeHeight();
    }
    void SigmaPiExpr::setContents(Expr *contents) {
        this->contents = contents;
        contents->parent = this;
        computeWidth();
        computeHeight();
    }
    SigmaPiExpr::~SigmaPiExpr() {
        DESTROY_IF_NONNULL(start);
        DESTROY_IF_NONNULL(finish);
        DESTROY_IF_NONNULL(contents);
    }
    void SigmaPiExpr::right(Expr *ex, Cursor &cursor) {
        if(ex == start || ex == finish) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void SigmaPiExpr::up(Expr *ex, Cursor &cursor) {
        if(ex == finish) {
            SAFE_EXEC(parent, up, this, cursor);
        }
        else if(ex == start) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(finish, getCursor, cursor, CURSORLOCATION_END);
        }
    }
    void SigmaPiExpr::down(Expr *ex, Cursor &cursor) {
        if(ex == finish) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_START);
        }
        else if(ex == start) {
            SAFE_EXEC(parent, down, this, cursor);
        }
        else {
            SAFE_EXEC(start, getCursor, cursor, CURSORLOCATION_END);
        }
    }
    void SigmaPiExpr::getCursor(Cursor &cursor, CursorLocation location) {
        SAFE_EXEC(contents, getCursor, cursor, location);
    }
    void SigmaPiExpr::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
        SAFE_EXEC(start, updatePosition, dx, dy);
        SAFE_EXEC(finish, updatePosition, dx, dy);
    }
}

#undef SAFE_EXEC
#undef SAFE_EXEC_0
#undef VERIFY_INBOUNDS
#undef ASSERT_NONNULL
#undef DESTROY_IF_NONNULL
