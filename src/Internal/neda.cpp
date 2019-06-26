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

    // *************************** Character ***************************************
    void Character::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        dest.drawImage(x, y, lcd::getChar(ch));
    }
    uint16_t Character::getWidth() {
        return lcd::getChar(ch).width;
    }
    uint16_t Character::getHeight() {
        return lcd::getChar(ch).height;
    }
    Character* Character::copy() {
        return new Character(ch);
    }
	
	// *************************** Expr ***************************************
    void Expr::draw(lcd::LCD12864 &dest) {
        draw(dest, x, y);
    }
    // Default impl: Call the parent's cursor method, if it has one
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
	
	// *************************** Container ***************************************
    uint16_t Container::getTopSpacing() {
        if(contents.length() == 0) {
            return EMPTY_EXPR_HEIGHT / 2;
        }
        uint16_t maxTopSpacing = 0;
        // Take the max of all the top spacings
        for(NEDAObj *ex : contents) {
            if(ex->getType() == ObjType::CHAR_TYPE) {
                maxTopSpacing = max(static_cast<uint16_t>(((Character*) ex)->getHeight() / 2), maxTopSpacing);
            }
            else {
                uint16_t ts = SAFE_EXEC_0((Expr*) ex, getTopSpacing);
                maxTopSpacing = max(ts, maxTopSpacing);
            }
        }
        return maxTopSpacing;
    }
	void Container::computeWidth() {
		// An empty Container has a default width and height
		if(contents.length() == 0) {
			exprWidth = EMPTY_EXPR_WIDTH;
            SAFE_EXEC(parent, computeWidth);
            return;
		}
		
		// Add up all the Expressions's widths
		exprWidth = 0;
        for(auto it = contents.begin(); it != contents.end(); it ++) {
            NEDAObj *ex = *it;
            if(ex->getType() == ObjType::CHAR_TYPE) {
                exprWidth += ((Character*) ex)->getWidth();
            }
            else {
                exprWidth += SAFE_ACCESS_0((Expr*) ex, exprWidth);
            }
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

        // Computing the height takes special logic as it is more than just taking the max of all the children's heights.
        // In the case of expressions such as 1^2+3_4, the height is greater than the max of all the children because the 1 and 3
        // have to line up.
		uint16_t maxTopSpacing = getTopSpacing();
		// Take the max of all the top spacings
		
        // Now with the max top spacing we can compute the heights and see what the max is
        uint16_t maxHeight = 0;
        for(NEDAObj *ex : contents) {
            // To calculate the height of any expression in this container, we essentially "replace" the top spacing with the max top
            // spacing, so that there is now a padding on the top. This is done in the expression below.
            // When that expression's top spacing is the max top spacing, the expression will be touching the top of the container.
            // Therefore, its height is just the height. In other cases, it will be increased by the difference between the max top
            // spacing and the top spacing.
            uint16_t height = ex->getType() == ObjType::CHAR_TYPE ? 
                    ((Character*) ex)->getHeight() - ((Character*) ex)->getHeight() / 2 + maxTopSpacing
                    : (SAFE_ACCESS_0((Expr*) ex, exprHeight) - SAFE_EXEC_0((Expr*) ex, getTopSpacing)) + maxTopSpacing;
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
            // Empty container shows up as a box
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

        // Special logic in drawing to make sure everything lines up
        uint16_t maxTopSpacing = getTopSpacing();

        for(auto it = contents.begin(); it != contents.end(); it ++) {
            NEDAObj *ex = *it;
            // Skip the expression if it's null
            if(!ex) {
                continue;
            }
            // For each expression, its top padding is the difference between the max top spacing and its top spacing.
            // E.g. A tall expression like 1^2 would have a higher top spacing than 3, so the max top spacing would be its top spacing;
            // So when drawing the 1^2, there is no difference between the max top spacing and the top spacing, and therefore it has
            // no top padding. But when drawing the 3, the difference between its top spacing and the max creates a top padding.
            if(ex->getType() == ObjType::CHAR_TYPE) {
                Character *ch = (Character*) ex;
                ch->draw(dest, x, y + (maxTopSpacing - ch->getHeight() / 2));
                x += ch->getWidth() + EXPR_SPACING;
            }
            else {
                Expr *expr = (Expr*) ex;
                expr->draw(dest, x, y + (maxTopSpacing - expr->getTopSpacing()));
                // Increase x so nothing overlaps
                x += expr->exprWidth + EXPR_SPACING;
            }
        }
    }
    void Container::recomputeHeights() {
        for(NEDAObj *ex : contents) {
            if(ex->getType() == ObjType::L_BRACKET || ex->getType() == ObjType::R_BRACKET || ex->getType() == ObjType::SUPERSCRIPT
                    || ex->getType() == ObjType::SUBSCRIPT) {
                ((Expr*) ex)->computeHeight();
            }
        }
    }
    void Container::add(NEDAObj *expr) {
        if(expr->getType() != ObjType::CHAR_TYPE) {
            ((Expr*) expr)->parent = this;
        }
        contents.add(expr);

        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    uint16_t Container::indexOf(NEDAObj *expr) {
        for(uint16_t i = 0; i < contents.length(); i ++) {
            if(contents[i] == expr) {
                return i;
            }
        }
        return 0xFFFF;
    }
    NEDAObj* Container::remove(uint16_t index) {
        NEDAObj *obj = contents[index];
        contents.removeAt(index);

        recomputeHeights();
        computeWidth();
        computeHeight();
        return obj;
    }
    void Container::addAt(uint16_t index, NEDAObj *exprToAdd) {
        contents.insert(exprToAdd, index);
        if(exprToAdd->getType() != ObjType::CHAR_TYPE) {
            ((Expr*) exprToAdd)->parent = this;
        }

        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    Container::~Container() {
        for(NEDAObj *ex : contents) {
            DESTROY_IF_NONNULL(ex);
        }
    }
    void Container::left(Expr *ex, Cursor &cursor) {
        // Check if the cursor is already in this expr
        if(!ex || cursor.expr == this) {
            if(cursor.index == 0) {
                SAFE_EXEC(parent, left, this, cursor);
            }
            else {
                // Check if we can go into the next expr
                --cursor.index;
                if(contents[cursor.index]->getType() != ObjType::CHAR_TYPE && contents[cursor.index]->getType() != ObjType::L_BRACKET 
                        && contents[cursor.index]->getType() != ObjType::R_BRACKET) {
                    ((Expr*) contents[cursor.index])->getCursor(cursor, CURSORLOCATION_END);
                }
            }
        }
        // Otherwise bring the cursor into this expr
        else {
            for(uint16_t i = 0; i < contents.length(); i ++) {
                // Find the expr the request came from
                if(contents[i] == ex) {
                    // Set the expr the cursor is in
                    cursor.expr = this;
                    // Set the index
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
            else {
                // If we can go into the next expr then do so
                if(cursor.index < contents.length() && contents[cursor.index]->getType() != ObjType::CHAR_TYPE
                    && contents[cursor.index]->getType() != ObjType::L_BRACKET && contents[cursor.index]->getType() != ObjType::R_BRACKET) {
                    ((Expr*) contents[cursor.index])->getCursor(cursor, CURSORLOCATION_START);
                }
                // Otherwise move the cursor
                else {
                    ++cursor.index;
                }
            }
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
    void Container::up(Expr *ex, Cursor &cursor) {
        if(ex != nullptr && ex->getType() == ObjType::SUBSCRIPT) {
            uint16_t index = indexOf(ex);
            if(index != 0xFFFF) {
                // Place the cursor in front of the subscript
                cursor.index = index;
                cursor.expr = this;
            }
        }
        else {
            // Find the nearest superscript
            for(auto it = contents.begin() + cursor.index; it != contents.end(); ++it) {
                NEDAObj *ex = *it;
                // Iterate until we hit an element that isn't a char
                if(ex->getType() != ObjType::CHAR_TYPE) {
                    // If it's a superscript then place the cursor into it and return
                    if(ex->getType() == ObjType::SUPERSCRIPT) {
                        ((Expr*) ex)->getCursor(cursor, CURSORLOCATION_START);
                        return;
                    }
                    // Otherwise break the loop
                    else {
                        break;
                    }
                }
            }

            // If none found then pass the call up
            SAFE_EXEC(parent, up, this, cursor);
        }
    }
    void Container::down(Expr *ex, Cursor &cursor) {
        if(ex != nullptr && ex->getType() == ObjType::SUPERSCRIPT) {
            uint16_t index = indexOf(ex);
            if(index != 0xFFFF) {
                // Place the cursor to the front of the superscript
                cursor.index = index;
                cursor.expr = this;
            }
        }
        else {
            // Find the nearest subscript
            for(auto it = contents.begin() + cursor.index; it != contents.end(); ++it) {
                NEDAObj *ex = *it;
                // Iterate until we hit an element that isn't a char
                if(ex->getType() != ObjType::CHAR_TYPE) {
                    // If it's a subscript then place the cursor into it and return
                    if(ex->getType() == ObjType::SUBSCRIPT) {
                        ((Expr*) ex)->getCursor(cursor, CURSORLOCATION_START);
                        return;
                    }
                    // Otherwise break the loop
                    else {
                        break;
                    }
                }
            }

            SAFE_EXEC(parent, down, this, cursor);
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
            cursorX += ((*it)->getType() == ObjType::CHAR_TYPE ? ((Character*) *it)->getWidth() : ((Expr*) *it)->exprWidth)
                    + EXPR_SPACING;
        }
        --cursorX;
        out.x = cursorX;
        out.width = 2;
        // Adjust the size and location of the cursor based on a reference element
        NEDAObj *ref;
        // If the cursor's index is not 0, then take the elem in front
        if(cursor.index != 0) {
            ref = contents[cursor.index - 1];
        }
        else {
            // Otherwise take the elem after if the container is not empty
            if(contents.length() > 0) {
                ref = contents[cursor.index];
            }
            // If container is empty, use default value and return
            else {
                out.y = y;
                out.height = EMPTY_EXPR_HEIGHT;
                return;
            }
        }

        if(ref->getType() == ObjType::CHAR_TYPE) {
        uint16_t maxTopSpacing = getTopSpacing();
            out.height = ((Character*) ref)->getHeight();
            out.y = y - ((Character*) ref)->getHeight() / 2 + maxTopSpacing;
        }
        else {
            out.height = ((Expr*) ref)->exprHeight;
            out.y = ((Expr*) ref)->y;
        }
    }
    void Container::drawCursor(lcd::LCD12864 &dest, const Cursor &cursor) {
        CursorInfo info;
        getCursorInfo(cursor, info);
        for(uint16_t i = 0; i < info.height; i ++) {
            dest.setPixel(info.x, info.y + i, true);
            dest.setPixel(info.x + 1, info.y + i, true);
        }
    }
    void Container::addAtCursor(NEDAObj *expr, Cursor &cursor) {
        contents.insert(expr, cursor.index);
        ++cursor.index;
        if(expr->getType() != ObjType::CHAR_TYPE) {
            ((Expr*) expr)->parent = this;
        }
        recomputeHeights();
        computeWidth();
        computeHeight();
    }
    // Returns the expression removed for deletion
    NEDAObj* Container::removeAtCursor(Cursor &cursor) {
        if(cursor.index != 0) {
            NEDAObj *obj = contents[--cursor.index];
            contents.removeAt(cursor.index);
            recomputeHeights();
            computeWidth();
            computeHeight();
            return obj;
        }
        return nullptr;
    }
    void Container::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
        for(NEDAObj *ex : contents) {
            if(ex->getType() != ObjType::CHAR_TYPE) {
                SAFE_EXEC((Expr*) ex, updatePosition, dx, dy);
            }
        }
    }
    Container* Container::copy() {
        Container *c = new Container();
        for(NEDAObj *ex : contents) {
            c->add(ex->copy());
        }
        
        return c;
    }
    void Container::addString(const char *str) {
        while(*str != '\0') {
            add(new Character(*(str++)));
        }
    }

    // *************************** Fraction ***************************************
    uint16_t Fraction::getTopSpacing() {
        // The top spacing of a fraction is equal to the height of its numerator, plus a pixel of spacing between the numerator and
        // the fraction line.
        return SAFE_ACCESS_0(numerator, exprHeight) + 1;
    }
	void Fraction::computeWidth() {
		// Take the greater of the widths and add 2 for the spacing at the sides
        uint16_t numeratorWidth = SAFE_ACCESS_0(numerator, exprWidth);
		uint16_t denominatorWidth = SAFE_ACCESS_0(denominator, exprWidth);
		exprWidth = max(numeratorWidth, denominatorWidth) + 2;
        SAFE_EXEC(parent, computeWidth);
	}
	void Fraction::computeHeight() {
		uint16_t numeratorHeight = SAFE_ACCESS_0(numerator, exprHeight);
		uint16_t denominatorHeight = SAFE_ACCESS_0(denominator, exprHeight);
		// Take the sum of the heights and add 3 for the fraction line
		exprHeight = numeratorHeight + denominatorHeight + 3;
        SAFE_EXEC(parent, computeHeight);
	}
	void Fraction::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);
		// Watch out for null pointers
        ASSERT_NONNULL(numerator);
        ASSERT_NONNULL(denominator);
		
		uint16_t width = exprWidth;
		// Center horizontally
		numerator->draw(dest, x + (width - numerator->exprWidth) / 2, y);
		uint16_t numHeight = numerator->exprHeight;
		for(uint16_t i = 0; i < width; i ++) {
			// Draw the fraction line
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
    Fraction* Fraction::copy() {
        return new Fraction((neda::Expr*) numerator->copy(), (neda::Expr*) denominator->copy());
    }
	
	// *************************** LeftBracket ***************************************
    uint16_t LeftBracket::getTopSpacing() {
        // Parent must be a Container
        if(!parent) {
            return 0xFFFF;
        }
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxSpacing = 0;
        // Used to find the end of the brackets
        uint16_t nesting = 1;
        for(auto it = parentContents.begin() + index + 1; it != parentContents.end(); ++ it) {
            NEDAObj *ex = *it;
            // Increase/Decrease the nesting depth if we see a bracket
            if(ex->getType() == ObjType::L_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ObjType::R_BRACKET) {
                nesting --;
                // Exit if nesting depth is 0
                if(!nesting) {
                    break;
                }
            }
            else {
                if(ex->getType() == ObjType::CHAR_TYPE) {
                    maxSpacing = max(maxSpacing, static_cast<uint16_t>(((Character*) ex)->getHeight() / 2));
                }
                else {
                    maxSpacing = max(maxSpacing, ((Expr*) ex)->getTopSpacing());
                }
            }
        }
        // If there is nothing after this left bracket, give it a default
        return maxSpacing ? maxSpacing : Container::EMPTY_EXPR_HEIGHT / 2;
    }
    void LeftBracket::computeWidth() {
        exprWidth = 3;
    }
    void LeftBracket::computeHeight() {
        if(!parent) {
            // Default
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
            return;
        }
        // Parent must be a Container
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxHeight = 0;
        uint16_t nesting = 1;
        uint16_t maxTopSpacing = getTopSpacing();
        for(auto it = parentContents.begin() + index + 1; it != parentContents.end(); ++ it) {
            NEDAObj *ex = *it;
            if(ex->getType() == ObjType::L_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ObjType::R_BRACKET) {
                nesting --;
                if(!nesting) {
                    break;
                }
            }
            else {
                if(ex->getType() == ObjType::CHAR_TYPE) {
                    maxHeight = max(maxHeight, static_cast<uint16_t>(((Character*) ex)->getHeight() - ((Character*) ex)->getHeight() / 2 + maxTopSpacing));
                }
                else {
                    maxHeight = max(maxHeight, static_cast<uint16_t>(((Expr*) ex)->exprHeight - ((Expr*) ex)->getTopSpacing() + maxTopSpacing));
                }
            }
        }
        // If there is nothing after this left bracket, give it a default
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
    LeftBracket* LeftBracket::copy() {
        return new LeftBracket();
    }

    // *************************** RightBracket ***************************************
    uint16_t RightBracket::getTopSpacing() {
        // Parent must be a Container
        if(!parent) {
            return 0xFFFF;
        }
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxSpacing = 0;
        // Used to find the end of the brackets
        uint16_t nesting = 1;
        // Iterate backwards
        for(auto it = parentContents.begin() + index - 1; it >= parentContents.begin(); -- it) {
            NEDAObj *ex = *it;
            // Increase/Decrease the nesting depth if we see a bracket
            if(ex->getType() == ObjType::R_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ObjType::L_BRACKET) {
                nesting --;
                // Exit if nesting depth is 0
                if(!nesting) {
                    break;
                }
            }
            else {
                if(ex->getType() == ObjType::CHAR_TYPE) {
                    maxSpacing = max(maxSpacing, static_cast<uint16_t>(((Character*) ex)->getHeight() / 2));
                }
                else {
                    maxSpacing = max(maxSpacing, ((Expr*) ex)->getTopSpacing());
                }
            }
        }
        // If there is nothing after this left bracket, give it a default
        return maxSpacing ? maxSpacing : Container::EMPTY_EXPR_HEIGHT / 2;
    }
    void RightBracket::computeWidth() {
        exprWidth = 3;
    }
    void RightBracket::computeHeight() {
        if(!parent) {
            // Default
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
            return;
        }
        // Parent must be a Container
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        uint16_t maxHeight = 0;
        uint16_t nesting = 1;
        uint16_t maxTopSpacing = getTopSpacing();
        // Iterate backwards
        for(auto it = parentContents.begin() + index - 1; it >= parentContents.begin(); -- it) {
            NEDAObj *ex = *it;
            if(ex->getType() == ObjType::R_BRACKET) {
                nesting ++;
            }
            else if(ex->getType() == ObjType::L_BRACKET) {
                nesting --;
                if(!nesting) {
                    break;
                }
            }
            else {
                if(ex->getType() == ObjType::CHAR_TYPE) {
                    maxHeight = max(maxHeight, static_cast<uint16_t>(((Character*) ex)->getHeight() - ((Character*) ex)->getHeight() / 2 + maxTopSpacing));
                }
                else {
                    maxHeight = max(maxHeight, static_cast<uint16_t>(((Expr*) ex)->exprHeight - ((Expr*) ex)->getTopSpacing() + maxTopSpacing));
                }
            }
        }
        // If there is nothing after this left bracket, give it a default
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
    RightBracket* RightBracket::copy() {
        return new RightBracket();
    }
	
	// *************************** Radical ***************************************
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
        // Move cursor to n only if the cursor is currently under the radical sign, and n is non-null
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
    Radical* Radical::copy() {
        return new Radical((neda::Expr*) contents->copy(), n ? (neda::Expr*) n->copy() : nullptr);
    }

    // *************************** Superscript ***************************************
    uint16_t Superscript::getTopSpacing() {
        if(!parent) {
            return SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT / 2 - OVERLAP;
        }
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        // Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            return SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT / 2 - OVERLAP;
        }
        NEDAObj *prevObj = parentContents[index - 1];
        uint16_t prevHeight = prevObj->getType() == ObjType::CHAR_TYPE ? ((Character*) prevObj)->getHeight() : ((Expr*) prevObj)->exprHeight;
        return SAFE_ACCESS_0(contents, exprHeight) + prevHeight / 2 - OVERLAP;
    }
    void Superscript::computeWidth() {
        exprWidth = SAFE_ACCESS_0(contents, exprWidth);
        SAFE_EXEC(parent, computeWidth);
    }
    void Superscript::computeHeight() {
        if(!parent) {
            exprHeight = SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT - OVERLAP;
            SAFE_EXEC(parent, computeHeight);
            return;
        }
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        // Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            exprHeight = SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT - OVERLAP;
            SAFE_EXEC(parent, computeHeight);
            return;
        }
        NEDAObj *prevObj = parentContents[index - 1];
        uint16_t prevHeight = prevObj->getType() == ObjType::CHAR_TYPE ? ((Character*) prevObj)->getHeight() : ((Expr*) prevObj)->exprHeight;
        exprHeight = SAFE_ACCESS_0(contents, exprHeight) + prevHeight - OVERLAP;

        SAFE_EXEC(parent, computeHeight);
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
    Superscript* Superscript::copy() {
        return new Superscript((neda::Expr*) contents->copy());
    }
	
	// *************************** Subscript ***************************************
	uint16_t Subscript::getTopSpacing() {
		if(!parent) {
            return Container::EMPTY_EXPR_HEIGHT / 2;
        }
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        // Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            return Container::EMPTY_EXPR_HEIGHT / 2;
        }
        NEDAObj *prevObj = parentContents[index - 1];
        uint16_t prevSpacing = prevObj->getType() == ObjType::CHAR_TYPE ? 
                ((Character*) prevObj)->getHeight() / 2
                : ((Expr*) prevObj)->getTopSpacing();
        return prevSpacing;
	}
	void Subscript::computeWidth() {
		exprWidth = SAFE_ACCESS_0(contents, exprWidth);
        SAFE_EXEC(parent, computeWidth);
	}
	void Subscript::computeHeight() {
		if(!parent) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT - OVERLAP + SAFE_ACCESS_0(contents, exprHeight);
            return;
        }
        Container *parentContainer = (Container*) parent;
        auto &parentContents = parentContainer->contents;
        uint16_t index = parentContainer->indexOf(this);
        // Look at the expression right before it. If there is no expression before, return the default
        if(index == 0) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT - OVERLAP + SAFE_ACCESS_0(contents, exprHeight);
            return;
        }
        NEDAObj *prevObj = parentContents[index - 1];
        uint16_t prevHeight = prevObj->getType() == ObjType::CHAR_TYPE ? ((Character*) prevObj)->getHeight() : ((Expr*) prevObj)->exprHeight;
        exprHeight = prevHeight - OVERLAP + SAFE_ACCESS_0(contents, exprHeight);
        SAFE_EXEC(parent, computeHeight);
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
    Subscript* Subscript::copy() {
        return new Subscript((neda::Expr*) contents->copy());
    }
	
	// *************************** SigmaPi ***************************************
	uint16_t SigmaPi::getTopSpacing() {
        // The top spacing of this expr can be split into two cases: when the contents are tall and when the contents are short.
        // When the contents are tall enough, the result is simply the top spacing of the contents (b)
        // Otherwise, it is the distance from the top to the middle of the base of the contents.
        // Therefore, we add up the heights of the expr at the top, the spacing, and the overlap between the symbol and the contents,
        // then subtract half of the base height of the contents (height - top spacing)
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
        // Top spacings - Taken from neda::SigmaPi::getTopSpacing()
        uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_ACCESS_0(contents, exprHeight) - SAFE_EXEC_0(contents, getTopSpacing));
        uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
        uint16_t maxTopSpacing = max(a, b);
        // Logic same as neda::Container::exprHeight
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
        
        // Top spacings - Taken from neda::SigmaPi::getTopSpacing()
        uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
                - (SAFE_ACCESS_0(contents, exprHeight) - SAFE_EXEC_0(contents, getTopSpacing));
        uint16_t b = SAFE_EXEC_0(contents, getTopSpacing);
        uint16_t maxTopSpacing = max(a, b);
        // Logic same as neda::Container::draw()
        uint16_t symbolYOffset = maxTopSpacing - a;
        uint16_t contentsYOffset = maxTopSpacing - b;

        // Center the top, the bottom and the symbol
        uint16_t widest = max(start->exprWidth, max(finish->exprWidth, symbol.width));
        finish->draw(dest, x + (widest - finish->exprWidth) / 2, y + symbolYOffset);
        dest.drawImage(x + (widest - symbol.width) / 2, y + finish->exprHeight + 2 + symbolYOffset, symbol);
        start->draw(dest, x + (widest - start->exprWidth) / 2, y + finish->exprHeight + 2 + symbol.height + 2 + symbolYOffset);

        contents->draw(dest, x + widest + 3, y + contentsYOffset);
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
    SigmaPi* SigmaPi::copy() {
        return new SigmaPi(symbol, (neda::Expr*) start->copy(), (neda::Expr*) finish->copy(), (neda::Expr*) contents->copy());
    }

    // *************************** Cursor ***************************************
    void Cursor::draw(lcd::LCD12864 &dest) {
        expr->drawCursor(dest, *this);
    }
    void Cursor::getInfo(CursorInfo &info) {
        expr->getCursorInfo(*this, info);
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
    void Cursor::add(NEDAObj *obj) {
        expr->addAtCursor(obj, *this);
    }

    // *************************** Misc ***************************************
    Container* makeString(const char *str) {
        Container *cont = new Container;
        while(*str != '\0') {
            cont->add(new Character(*(str++)));
        }
        return cont;
    }
}

#undef VERIFY_INBOUNDS
#undef ASSERT_NONNULL
#undef DESTROY_IF_NONNULL
