#include "neda.hpp"
#include "util.hpp"
#include "lcd12864_charset.hpp"

#define VERIFY_INBOUNDS(x, y) if(x >= 128 || y >= 64 || x + exprWidth < 0 || y + exprHeight < 0) return
#define ASSERT_NONNULL(obj) if(!obj) return
#define DESTROY_IF_NONNULL(obj) if(obj) delete obj

/*
* NEDA: Nested Expression Display Algorithm
* 
* This is the math display engine for the calculator.
* 
* WARNING: To prevent possible memory leaks, all NEDA classes have destructors that deletes all its children.
* This means that all instances have to be allocated on the heap, with the new operator to prevent segmentation faults.
* If allocated on the stack, the variable must never go out of scope.
* DO NOT ALLOCATE WITH malloc
*/
namespace neda {
	
	//*************************** Expr ***************************************
    void Expr::draw(lcd::LCD12864 &dest) {
        draw(dest, x, y);
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
    Expr* Expr::getTopLevel() {
        return parent ? parent->getTopLevel() : this;
    }
    void Expr::updatePosition(int16_t dx, int16_t dy) {
        x += dx;
        y += dy;
    }
	
	//*************************** String ***************************************
    uint16_t String::getTopSpacing() {
        //Because of its basic nature, String doesn't need any special processing
        return exprHeight / 2;
    }
	void String::computeWidth() {
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
	void String::computeHeight() {
        if(contents->length() == 0) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
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
	void String::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
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
	void String::addChar(char ch) {
		contents->add(ch);
		computeWidth();
		computeHeight();
	}
    uint16_t String::length() {
        return contents->length();
    }
    void String::addAtCursor(char ch, Cursor &cursor) {
        contents->insert(ch, cursor.index);
		cursor.index ++;
		computeWidth();
		computeHeight();
    }
    void String::removeAtCursor(Cursor &cursor) {
        if(cursor.index != 0) {
            contents->removeAt(--cursor.index);
        }
		computeWidth();
		computeHeight();
    }
    void String::drawCursor(lcd::LCD12864 &dest, const Cursor &cursor) {
        CursorInfo info;
        getCursorInfo(cursor, info);
        for(int16_t i = 0; i < exprHeight; i ++) {
            dest.setPixel(info.x, y + i, true);
            dest.setPixel(info.x + 1, y + i, true);
        }
    }
    String* String::beforeCursor(const Cursor &cursor) {
        DynamicArray<char> *a = new DynamicArray<char>(contents->begin(), contents->begin() + cursor.index);
        return new String(a);
    }
    String* String::afterCursor(const Cursor &cursor) {
        DynamicArray<char> *b = new DynamicArray<char>(contents->begin() + cursor.index, contents->end());
        return new String(b);
    }
    void String::merge(const String *other) {
        contents->merge(other->contents);
        computeWidth();
        computeHeight();
    }
    void String::getCursorInfo(const Cursor &cursor, CursorInfo &out) {
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
    String::~String() {
        delete contents;
    }
    void String::left(Expr *ex, Cursor &cursor) {
        if(cursor.index == 0) {
            SAFE_EXEC(parent, left, this, cursor);
        }
        else {
            cursor.index --;
        }
    }
    void String::right(Expr *ex, Cursor &cursor) {
        if(cursor.index == contents->length()) {
            SAFE_EXEC(parent, right, this, cursor);
        }
        else {
            cursor.index ++;
        }
    }
    void String::getCursor(Cursor &cursor, CursorLocation location) {
        cursor.expr = this;
        cursor.index = location == CURSORLOCATION_START ? 0 : contents->length();
    }
    void String::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
    }
    bool String::isEmptyString(Expr *str) {
        return str->getType() == ExprType::STRING && ((String*) str)->contents->length() == 0;
    }
	
	//*************************** Container ***************************************
    uint16_t Container::getTopSpacing() {
        uint16_t maxTopSpacing = 0;
        //Take the max of all the top spacings
        for(Expr *ex : contents) {
            uint16_t ts = SAFE_EXEC_0(ex, getTopSpacing);
            maxTopSpacing = max(ts, maxTopSpacing);
        }
        return maxTopSpacing;
    }
	void Container::computeWidth() {
		//An empty Container has a default width and height
		if(contents.length() == 0) {
			exprWidth = EMPTY_EXPR_WIDTH;
            SAFE_EXEC(parent, computeWidth);
            return;
		}
		
		//Add up all the Expressions's widths
		exprWidth = 0;
        for(auto it = contents.begin(); it != contents.end(); it ++) {
            Expr *ex = *it;
            exprWidth += SAFE_ACCESS_0(ex, exprWidth);
        }
        exprWidth += max(0, (contents.length() - 1) * EXPR_SPACING);
        SAFE_EXEC(parent, computeWidth);
	}
	void Container::computeHeight() {
		if(contents.length() == 0) {
			exprHeight = EMPTY_EXPR_HEIGHT;
            SAFE_EXEC(parent, computeHeight);
			return;
		}

        //Computing the height takes special logic as it is more than just taking the max of all the children's heights.
        //In the case of expressions such as 1^2+3_4, the height is greater than the max of all the children because the 1 and 3
        //have to line up.
		uint16_t maxTopSpacing = getTopSpacing();
		//Take the max of all the top spacings
		
        //Now with the max top spacing we can compute the heights and see what the max is
        uint16_t maxHeight = 0;
        for(Expr *ex : contents) {
            //To calculate the height of any expression in this container, we essentially "replace" the top spacing with the max top
            //spacing, so that there is now a padding on the top. This is done in the expression below.
            //When that expression's top spacing is the max top spacing, the expression will be touching the top of the container.
            //Therefore, its height is just the height. In other cases, it will be increased by the difference between the max top
            //spacing and the top spacing.
            uint16_t height = (SAFE_ACCESS_0(ex, exprHeight) - SAFE_EXEC_0(ex, getTopSpacing)) + maxTopSpacing;
            maxHeight = max(height, maxHeight);
        }
        exprHeight = maxHeight;
        SAFE_EXEC(parent, computeHeight);
    }
    void Container::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);

        if(contents.length() == 0) {
            //Empty container shows up as a box
            for(uint16_t w = 0; w < exprWidth; w ++) {
                dest.setPixel(x + w, y, true);
                dest.setPixel(x + w, y + exprHeight - 1, true);
            }
            for(uint16_t h = 0; h < exprHeight; h ++) {
                dest.setPixel(x, y + h, true);
                dest.setPixel(x + exprWidth - 1, y + h, true);
            }
            //Special handling for empty String: the x and y still have to be set
            if(contents.length() == 1) {
                contents[0]->draw(dest, x, y);
            }
            return;
        }

        //Special logic in drawing to make sure everything lines up
        uint16_t maxTopSpacing = getTopSpacing();

        for(auto it = contents.begin(); it != contents.end(); it ++) {
            Expr *ex = *it;
            //Skip the expression if it's null
            if(!ex) {
                continue;
            }
            //For each expression, its top padding is the difference between the max top spacing and its top spacing.
            //E.g. A tall expression like 1^2 would have a higher top spacing than 3, so the max top spacing would be its top spacing;
            //So when drawing the 1^2, there is no difference between the max top spacing and the top spacing, and therefore it has
            //no top padding. But when drawing the 3, the difference between its top spacing and the max creates a top padding.
            ex->draw(dest, x, y + (maxTopSpacing - ex->getTopSpacing()));
            //Increase x so nothing overlaps
            x += ex->exprWidth + EXPR_SPACING;
        }
    }
    void Container::recomputeHeights() {
        for(Expr *ex : contents) {
            if(ex->getType() == ExprType::L_BRACKET || ex->getType() == ExprType::R_BRACKET || ex->getType() == ExprType::SUPERSCRIPT
                    || ex->getType() == ExprType::SUBSCRIPT) {
                ex->computeHeight();
            }
        }
    }
    void Container::addExpr(Expr *expr) {
        expr->parent = this;
        contents.add(expr);

        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    uint16_t Container::indexOf(Expr *expr) {
        for(uint16_t i = 0; i < contents.length(); i ++) {
            if(contents[i] == expr) {
                return i;
            }
        }
        return 0xFFFF;
    }
    void Container::removeExpr(uint16_t index) {
        contents.removeAt(index);

        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    void Container::replaceExpr(uint16_t index, Expr *replacement) {
        contents[index] = replacement;
        replacement->parent = this;

        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    void Container::addAt(uint16_t index, Expr *exprToAdd) {
        contents.insert(exprToAdd, index);
        exprToAdd->parent = this;

        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    DynamicArray<Expr*>* Container::getContents() {
        return &contents;
    }
    Container::~Container() {
        for(Expr *ex : contents) {
            DESTROY_IF_NONNULL(ex);
        }
    }
    void Container::left(Expr *ex, Cursor &cursor) {
        //Check if the cursor is already in this expr
        if(!ex || cursor.expr == this) {
            if(cursor.index == 0) {
                SAFE_EXEC(parent, left, this, cursor);
            }
            contents[cursor.index - 1]->getCursor(cursor, CURSORLOCATION_END);
        }
        //Otherwise bring the cursor into this expr
        else {
            for(uint16_t i = 0; i < contents.length(); i ++) {
                //Find the expr the request came from
                if(contents[i] == ex) {
                    //Set the expr the cursor is in
                    cursor.expr = this;
                    //Set the index
                    cursor.index = i;
                    break;
                }
            }
        }
        
    }
    void Container::right(Expr *ex, Cursor &cursor) {
        if(!ex || cursor.expr == this) {
            if(cursor.index == contents.length()) {
                SAFE_EXEC(parent, right, this, cursor);
            }
            contents[cursor.index]->getCursor(cursor, CURSORLOCATION_START);
        }
        else {
            for(uint16_t i = 0; i < contents.length(); i ++) {
                if(contents[i] == ex) {
                    cursor.expr = this;
                    cursor.index = i + 1;
                    break;
                }
            }
        }
    }
    void Container::getCursor(Cursor &cursor, CursorLocation location) {
        cursor.expr = this;
        cursor.index = location == CURSORLOCATION_START ? 0 : contents.length();
    }
    void Container::getCursorInfo(const Cursor &cursor, CursorInfo &out) {
        int16_t cursorX = x;
        uint16_t i = 0;
        for(auto it = contents.begin(); it != contents.end() && i < cursor.index; ++it, ++i) {
            cursorX += (*it)->exprWidth + EXPR_SPACING;
        }
        out.x = cursorX;
        out.y = y;
        out.width = 2;
        out.height = exprHeight;
    }
    void Container::drawCursor(lcd::LCD12864 &dest, const Cursor &cursor) {
        CursorInfo info;
        getCursorInfo(cursor, info);
        for(uint16_t i = 0; i < info.height; i ++) {
            dest.setPixel(info.x, info.y + i, true);
            dest.setPixel(info.x + 1, info.y + i, true);
        }
    }
    void Container::addAtCursor(Expr *expr, Cursor &cursor) {
        contents.insert(expr, cursor.index);
        ++cursor.index;
        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    void Container::removeAtCursor(Cursor &cursor) {
        if(cursor.index != 0) {
            contents.removeAt(--cursor.index);
        }
        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    void Container::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        for(Expr *ex : contents) {
            SAFE_EXEC(ex, updatePosition, dx, dy);
        }
    }

    //*************************** Fraction ***************************************
    uint16_t Fraction::getTopSpacing() {
        //The top spacing of a fraction is equal to the height of its numerator, plus a pixel of spacing between the numerator and
        //the fraction line.
        return SAFE_ACCESS_0(numerator, exprHeight) + 1;
    }
	void Fraction::computeWidth() {
		//Take the greater of the widths and add 2 for the spacing at the sides
        uint16_t numeratorWidth = SAFE_ACCESS_0(numerator, exprWidth);
		uint16_t denominatorWidth = SAFE_ACCESS_0(denominator, exprWidth);
		exprWidth = max(numeratorWidth, denominatorWidth) + 2;
        SAFE_EXEC(parent, computeWidth);
	}
	void Fraction::computeHeight() {
		uint16_t numeratorHeight = SAFE_ACCESS_0(numerator, exprHeight);
		uint16_t denominatorHeight = SAFE_ACCESS_0(denominator, exprHeight);
		//Take the sum of the heights and add 3 for the fraction line
		exprHeight = numeratorHeight + denominatorHeight + 3;
        SAFE_EXEC(parent, computeHeight);
	}
	void Fraction::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		//Watch out for null pointers
        ASSERT_NONNULL(numerator);
        ASSERT_NONNULL(denominator);
		
		uint16_t width = exprWidth;
		//Center horizontally
		numerator->draw(dest, x + (width - numerator->exprWidth) / 2, y);
		uint16_t numHeight = numerator->exprHeight;
		for(uint16_t i = 0; i < width; i ++) {
			//Draw the fraction line
			dest.setPixel(x + i, y + numHeight + 1, true);
		}
		denominator->draw(dest, x + (width - denominator->exprWidth) / 2, y + numHeight + 3);
	}
	Expr* Fraction::getNumerator() {
		return numerator;
	}
	Expr* Fraction::getDenominator() {
		return denominator;
	}
	void Fraction::setNumerator(Expr *numerator) {
		this->numerator = numerator;
        numerator->parent = this;
		computeWidth();
		computeHeight();
	}
	void Fraction::setDenominator(Expr *denominator) {
		this->denominator = denominator;
        denominator->parent = this;
		computeWidth();
		computeHeight();
	}
	Fraction::~Fraction() {
        DESTROY_IF_NONNULL(numerator);
        DESTROY_IF_NONNULL(denominator);
	}
    void Fraction::up(Expr *ex, Cursor &cursor) {
        if(ex == denominator) {
            SAFE_EXEC(numerator, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, up, this, cursor);
        }
    }
    void Fraction::down(Expr *ex, Cursor &cursor) {
        if(ex == numerator) {
            SAFE_EXEC(denominator, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, down, this, cursor);
        }
    }
    void Fraction::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            SAFE_EXEC(numerator, getCursor, cursor, location);
        }
        else {
            SAFE_EXEC(denominator, getCursor, cursor, location);
        }
    }
    void Fraction::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(numerator, updatePosition, dx, dy);
        SAFE_EXEC(denominator, updatePosition, dx, dy);
    }
	
	//*************************** LeftBracket ***************************************
    uint16_t LeftBracket::getTopSpacing() {
        //Parent must be a Container
        if(!parent) {
            return 0xFFFF;
        }
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxSpacing = 0;
        //Used to find the end of the brackets
        uint16_t nesting = 1;
        for(auto it = parentContents->begin() + index + 1; it != parentContents->end(); ++ it) {
            Expr *ex = *it;
            //Increase/Decrease the nesting depth if we see a bracket
            if(ex->getType() == ExprType::L_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ExprType::R_BRACKET) {
                nesting --;
                //Exit if nesting depth is 0
                if(!nesting) {
                    break;
                }
            }
            else {
                maxSpacing = max(maxSpacing, ex->getTopSpacing());
            }
        }
        //If there is nothing after this left bracket, give it a default
        return maxSpacing ? maxSpacing : Container::EMPTY_EXPR_HEIGHT / 2;
    }
    void LeftBracket::computeWidth() {
        exprWidth = 3;
    }
    void LeftBracket::computeHeight() {
        if(!parent) {
            //Default
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
            return;
        }
        //Parent must be a Container
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxHeight = 0;
        uint16_t nesting = 1;
        for(auto it = parentContents->begin() + index + 1; it != parentContents->end(); ++ it) {
            Expr *ex = *it;
            if(ex->getType() == ExprType::L_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ExprType::R_BRACKET) {
                nesting --;
                if(!nesting) {
                    break;
                }
            }
            else {
                maxHeight = max(maxHeight, ex->exprHeight);
            }
        }
        //If there is nothing after this left bracket, give it a default
        exprHeight = maxHeight ? maxHeight : Container::EMPTY_EXPR_HEIGHT;
    }
    void LeftBracket::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
        
        dest.setPixel(x + 2, y, true);
        dest.setPixel(x + 1, y + 1, true);
        for(uint16_t i = 2; i < exprHeight - 2; i ++) {
            dest.setPixel(x, y + i, true);
        }
        dest.setPixel(x + 1, y + exprHeight - 1 - 1, true);
        dest.setPixel(x + 2, y + exprHeight - 1, true);
    }

    //*************************** RightBracket ***************************************
    uint16_t RightBracket::getTopSpacing() {
        //Parent must be a Container
        if(!parent) {
            return 0xFFFF;
        }
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxSpacing = 0;
        //Used to find the end of the brackets
        uint16_t nesting = 1;
        //Iterate backwards
        for(auto it = parentContents->begin() + index - 1; it >= parentContents->begin(); -- it) {
            Expr *ex = *it;
            //Increase/Decrease the nesting depth if we see a bracket
            if(ex->getType() == ExprType::R_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ExprType::L_BRACKET) {
                nesting --;
                //Exit if nesting depth is 0
                if(!nesting) {
                    break;
                }
            }
            else {
                maxSpacing = max(maxSpacing, ex->getTopSpacing());
            }
        }
        //If there is nothing after this left bracket, give it a default
        return maxSpacing ? maxSpacing : Container::EMPTY_EXPR_HEIGHT / 2;
    }
    void RightBracket::computeWidth() {
        exprWidth = 3;
    }
    void RightBracket::computeHeight() {
        if(!parent) {
            //Default
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
            return;
        }
        //Parent must be a Container
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxHeight = 0;
        uint16_t nesting = 1;
        //Iterate backwards
        for(auto it = parentContents->begin() + index - 1; it >= parentContents->begin(); -- it) {
            Expr *ex = *it;
            if(ex->getType() == ExprType::R_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ExprType::L_BRACKET) {
                nesting --;
                if(!nesting) {
                    break;
                }
            }
            else {
                maxHeight = max(maxHeight, ex->exprHeight);
            }
        }
        //If there is nothing after this left bracket, give it a default
        exprHeight = maxHeight ? maxHeight : Container::EMPTY_EXPR_HEIGHT;
    }
    void RightBracket::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);

        dest.setPixel(x, y, true);
        dest.setPixel(x + 1, y + 1, true);
        for(uint16_t i = 2; i < exprHeight - 2; i ++) {
            dest.setPixel(x + 2, y + i, true);
        }
        dest.setPixel(x + 1, y + exprHeight - 1 - 1, true);
        dest.setPixel(x, y + exprHeight - 1, true);
    }
	
	//*************************** Radical ***************************************
	uint16_t Radical::getTopSpacing() {
		if(!n) { 
			return SAFE_EXEC_0(contents, getTopSpacing) + 2;
		}
		uint16_t regularTopSpacing = SAFE_EXEC_0(contents, getTopSpacing) + 2;
		return regularTopSpacing + max(0, n->exprHeight - CONTENTS_N_OVERLAP);
	}
	void Radical::computeWidth() {
		if(!n) {
			exprWidth = SAFE_ACCESS_0(contents, exprWidth) + 8;
            SAFE_EXEC(parent, computeWidth);
			return;
		}
		exprWidth = max(0, n->exprWidth - SIGN_N_OVERLAP) + SAFE_ACCESS_0(contents, exprWidth) + 8;
        SAFE_EXEC(parent, computeWidth);
	}
	void Radical::computeHeight() {
		if(!n) {
			exprHeight = SAFE_ACCESS_0(contents, exprHeight) + 2;
            SAFE_EXEC(parent, computeHeight);
			return;
		}
		exprHeight = max(0, n->exprHeight - CONTENTS_N_OVERLAP) + SAFE_ACCESS_0(contents, exprHeight) + 2;
        SAFE_EXEC(parent, computeHeight);
	}
	void Radical::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		if(!n) {
			ASSERT_NONNULL(contents);
			dest.drawLine(x, y + exprHeight - 1 - 2, x + 2, y + exprHeight - 1);
			dest.drawLine(x + 2, y + exprHeight - 1, x + 6, y);
			dest.drawLine(x + 6, y, x + exprWidth - 1, y);
			
			contents->draw(dest, x + 7, y + 2);
		}
		else {
			n->draw(dest, x, y);
			uint16_t xoffset = max(0, n->exprWidth - SIGN_N_OVERLAP);
			uint16_t yoffset = max(0, n->exprHeight - CONTENTS_N_OVERLAP);
			dest.drawLine(x + xoffset, y + exprHeight - 1 - 2, x + 2 + xoffset, y + exprHeight - 1);
			dest.drawLine(x + 2 + xoffset, y + exprHeight - 1, x + 6 + xoffset, y + yoffset);
			dest.drawLine(x + 6 + xoffset, y + yoffset, x + exprWidth - 1, y + yoffset);
			
			contents->draw(dest, x + 7 + xoffset, y + 2 + yoffset);
		}
	}
	Expr* Radical::getContents() {
		return contents;
	}
	Expr* Radical::getN() {
		return n;
	}
	void Radical::setContents(Expr *contents) {
		this->contents = contents;
        contents->parent = this;
		computeWidth();
		computeHeight();
	}
	void Radical::setN(Expr *n) {
		this->n = n;
        n->parent = this;
		computeWidth();
		computeHeight();
	}
	Radical::~Radical() {
        DESTROY_IF_NONNULL(contents);
        DESTROY_IF_NONNULL(n);
	}
    void Radical::left(Expr *ex, Cursor &cursor) {
        //Move cursor to n only if the cursor is currently under the radical sign, and n is non-null
        if(ex == contents && n) {
            SAFE_EXEC(n, getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, left, this, cursor);
        }
    }
    void Radical::right(Expr *ex, Cursor &cursor) {
        if(ex == n) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void Radical::getCursor(Cursor &cursor, CursorLocation location) {
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
    void Radical::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
        SAFE_EXEC(n, updatePosition, dx, dy);
    }

    //*************************** Superscript ***************************************
    uint16_t Superscript::getTopSpacing() {
        if(!parent) {
            return SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT - OVERLAP;
        }
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        //Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            return SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT - OVERLAP;
        }
        return SAFE_ACCESS_0(contents, exprHeight) + (*parentContents)[index - 1]->exprHeight / 2 - OVERLAP;
    }
    void Superscript::computeWidth() {
        exprWidth = SAFE_ACCESS_0(contents, exprWidth);
    }
    void Superscript::computeHeight() {
        exprHeight = SAFE_ACCESS_0(contents, exprHeight);
    }
    void Superscript::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
        SAFE_EXEC(contents, draw, dest, x, y);
    }
    void Superscript::setContents(Expr *contents) {
        this->contents = contents;
        computeWidth();
        computeHeight();
    }
    Superscript::~Superscript() {
        DESTROY_IF_NONNULL(contents);
    }
    void Superscript::updatePosition(int16_t dx, int16_t dy) {
        x += dx;
        y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
    }
    void Superscript::getCursor(Cursor &cursor, CursorLocation location) {
        SAFE_EXEC(contents, getCursor, cursor, location);
    }
	
	//*************************** Subscript ***************************************
	uint16_t Subscript::getTopSpacing() {
		if(!parent) {
            return Container::EMPTY_EXPR_HEIGHT / 2;
        }
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        //Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            return Container::EMPTY_EXPR_HEIGHT / 2;
        }
        return (*parentContents)[index - 1]->exprHeight / 2;
	}
	void Subscript::computeWidth() {
		exprWidth = SAFE_ACCESS_0(contents, exprWidth);
	}
	void Subscript::computeHeight() {
		if(!parent) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT - OVERLAP + SAFE_ACCESS_0(contents, exprHeight);
            return;
        }
        Container *parentContainer = (Container*) parent;
        auto parentContents = parentContainer->getContents();
        uint16_t index = parentContainer->indexOf(this);
        //Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT - OVERLAP + SAFE_ACCESS_0(contents, exprHeight);
            return;
        }
        exprHeight = (*parentContents)[index - 1]->exprHeight - OVERLAP + SAFE_ACCESS_0(contents, exprHeight);
	}
	void Subscript::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		if (!contents) {
			return;
		}
		contents->draw(dest, x, y + exprHeight - contents->exprHeight);
	}
	void Subscript::setContents(Expr *contents) {
		this->contents = contents;
        contents->parent = this;
		computeWidth();
		computeHeight();
	}
	Subscript::~Subscript() {
        DESTROY_IF_NONNULL(contents);
	}
    void Subscript::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            SAFE_EXEC(contents, getCursor, cursor, location);
        }
        else {
            SAFE_EXEC(contents, getCursor, cursor, location);
        }
    }
    void Subscript::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
    }
	
	//*************************** SigmaPi ***************************************
	uint16_t SigmaPi::getTopSpacing() {
        //The top spacing of this expr can be split into two cases: when the contents are tall and when the contents are short.
        //When the contents are tall enough, the result is simply the top spacing of the contents (b)
        //Otherwise, it is the distance from the top to the middle of the base of the contents.
        //Therefore, we add up the heights of the expr at the top, the spacing, and the overlap between the symbol and the contents,
        //then subtract half of the base height of the contents (height - top spacing)
		uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_ACCESS_0(contents, exprHeight) - SAFE_EXEC_0(contents, getTopSpacing));
		uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
		return max(a, b);
	}
	void SigmaPi::computeWidth() {
		uint16_t topWidth = SAFE_ACCESS_0(finish, exprWidth);
		uint16_t bottomWidth = SAFE_ACCESS_0(start, exprWidth);
		exprWidth = max(symbol.width, max(topWidth, bottomWidth)) + 3 + SAFE_ACCESS_0(contents, exprWidth);
        SAFE_EXEC(parent, computeWidth);
	}
	void SigmaPi::computeHeight() {
        //Top spacings - Taken from neda::SigmaPi::getTopSpacing()
        uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_ACCESS_0(contents, exprHeight) - SAFE_EXEC_0(contents, getTopSpacing));
        uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
        uint16_t maxTopSpacing = max(a, b);
        //Logic same as neda::Container::exprHeight
        uint16_t symbolHeight = SAFE_ACCESS_0(finish, exprHeight) + 2 + symbol.height + 2 + SAFE_ACCESS_0(start, exprHeight)
                + maxTopSpacing - a;
        uint16_t bodyHeight = SAFE_ACCESS_0(contents, exprHeight) + maxTopSpacing - b;

		exprHeight = max(symbolHeight, bodyHeight);
        SAFE_EXEC(parent, computeHeight);
	}
	void SigmaPi::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		ASSERT_NONNULL(start);
        ASSERT_NONNULL(finish);
        ASSERT_NONNULL(contents);
        
        //Top spacings - Taken from neda::SigmaPi::getTopSpacing()
        uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_ACCESS_0(contents, exprHeight) - SAFE_EXEC_0(contents, getTopSpacing));
        uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
        uint16_t maxTopSpacing = max(a, b);
        //Logic same as neda::Container::draw()
        uint16_t symbolYOffset = maxTopSpacing - a;
        uint16_t contentsYOffset = maxTopSpacing - b;

        //Center the top, the bottom and the symbol
        uint16_t widest = max(start->exprWidth, max(finish->exprWidth, symbol.width));
        finish->draw(dest, x + (widest - finish->exprWidth) / 2, y + symbolYOffset);
        dest.drawImage(x + (widest - symbol.width) / 2, y + finish->exprHeight + 2 + symbolYOffset, symbol);
        start->draw(dest, x + (widest - start->exprWidth) / 2, y + finish->exprHeight + 2 + symbol.height + 2 + symbolYOffset);

        contents->draw(dest, widest + 3, y + contentsYOffset);
	}
    Expr* SigmaPi::getStart() {
        return start;
    }
    Expr* SigmaPi::getFinish() {
        return finish;
    }
    Expr* SigmaPi::getContents() {
        return contents;
    }
    void SigmaPi::setStart(Expr *start) {
        this->start = start;
        start->parent = this;
        computeWidth();
        computeHeight();
    }
    void SigmaPi::setFinish(Expr *finish) {
        this->finish = finish;
        finish->parent = this;
        computeWidth();
        computeHeight();
    }
    void SigmaPi::setContents(Expr *contents) {
        this->contents = contents;
        contents->parent = this;
        computeWidth();
        computeHeight();
    }
    SigmaPi::~SigmaPi() {
        DESTROY_IF_NONNULL(start);
        DESTROY_IF_NONNULL(finish);
        DESTROY_IF_NONNULL(contents);
    }
    void SigmaPi::right(Expr *ex, Cursor &cursor) {
        if(ex == start || ex == finish) {
            SAFE_EXEC(contents, getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void SigmaPi::up(Expr *ex, Cursor &cursor) {
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
    void SigmaPi::down(Expr *ex, Cursor &cursor) {
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
    void SigmaPi::getCursor(Cursor &cursor, CursorLocation location) {
        SAFE_EXEC(contents, getCursor, cursor, location);
    }
    void SigmaPi::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        SAFE_EXEC(contents, updatePosition, dx, dy);
        SAFE_EXEC(start, updatePosition, dx, dy);
        SAFE_EXEC(finish, updatePosition, dx, dy);
    }

    //*************************** Cursor ***************************************
    void Cursor::draw(lcd::LCD12864 &dest) {
        if(expr->getType() == ExprType::STRING) {
            ((String*) expr)->drawCursor(dest, *this);
        }
        else if(expr->getType() == ExprType::CONTAINER) {
            ((Container*) expr)->drawCursor(dest, *this);
        }
    }
    void Cursor::getInfo(CursorInfo &info) {
        if(expr->getType() == ExprType::STRING) {
            ((String*) expr)->getCursorInfo(*this, info);
        }
        else if(expr->getType() == ExprType::CONTAINER) {
            ((Container*) expr)->getCursorInfo(*this, info);
        }
    }
    void Cursor::left() {
        expr->left(nullptr, *this);
    }
    void Cursor::right() {
        expr->right(nullptr, *this);
    }
    void Cursor::up() {
        expr->up(nullptr, *this);
    }
    void Cursor::down() {
        expr->down(nullptr, *this);
    }
}

#undef VERIFY_INBOUNDS
#undef ASSERT_NONNULL
#undef DESTROY_IF_NONNULL
