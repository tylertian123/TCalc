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
	uint16_t Character::getWidth() const {
		return lcd::getChar(ch).width;
	}
	uint16_t Character::getHeight() const {
		return lcd::getChar(ch).height;
	}
    const lcd::Image& Character::getCharData() const {
        return lcd::getChar(ch);
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
    void Container::computeDimensions() {
        // If this expression is empty, return the default values
        if(contents.length() == 0) {
			topSpacing = EMPTY_EXPR_HEIGHT / 2;
            exprWidth = EMPTY_EXPR_WIDTH;
            exprHeight = EMPTY_EXPR_HEIGHT;
		}
        else {
            // The top spacing is the util::max of all the top spacings
            topSpacing = 0;
            // The expression width is increased with each element
            exprWidth = 0;
            // Loop through all elements
            for(NEDAObj *ex : contents) {
                // Separate processing for regular characters vs expressions
                if(ex->getType() == ObjType::CHAR_TYPE) {
                    auto charData = static_cast<Character*>(ex)->getCharData();
                    // Take the maximum of the current util::max and the top spacing
                    topSpacing = util::max(static_cast<uint16_t>(charData.height / 2), topSpacing);
                    // Add the expression width to the total width
                    exprWidth += charData.width;
                }
                else {
                    Expr *expr = static_cast<Expr*>(ex);
                    // Take the maximum of the current util::max and the top spacing
                    topSpacing = util::max(static_cast<uint16_t>(SAFE_ACCESS_0(expr, topSpacing)), topSpacing);
                    // Add the expression width to the total width
                    exprWidth += SAFE_ACCESS_0(expr, exprWidth);
                }
            }
            // Add the gaps between elements
            // Since content length must be nonzero this cannot be negative
            exprWidth += (contents.length() - 1) * EXPR_SPACING;

            // The expression height is computed separately as it requires the top spacing
            // Computing the height takes special logic as it is more than just taking the util::max of all the children's heights.
            // In the case of expressions such as 1^2+3_4, the height is greater than the util::max of all the children because the 
            // 1 and 3 have to line up.
            // Now with the util::max top spacing we can compute the heights and see what the util::max is
            exprHeight = 0;
            for(NEDAObj *ex : contents) {
                // To calculate the height of any expression in this container, we essentially "replace" the top spacing with the util::max
                // top spacing, so that there is now a padding on the top. This is done in the expression below.
                // When that expression's top spacing is the util::max top spacing, the expression will be touching the top of the container.
                // Therefore, its height is just the height. In other cases, it will be increased by the difference between the util::max top
                // spacing and the top spacing.
                uint16_t height = ex->getType() == ObjType::CHAR_TYPE ? 
                        ((Character*) ex)->getHeight() - ((Character*) ex)->getHeight() / 2 + topSpacing
                        : (SAFE_ACCESS_0((Expr*) ex, exprHeight) - SAFE_ACCESS_0((Expr*) ex, topSpacing)) + topSpacing;
                exprHeight = util::max(height, exprHeight);
            }
        }

        SAFE_EXEC(parent, computeDimensions);
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

		for(auto it = contents.begin(); it != contents.end(); it ++) {
			NEDAObj *ex = *it;
			// Skip the expression if it's null
			if(!ex) {
				continue;
			}
			// For each expression, its top padding is the difference between the util::max top spacing and its top spacing.
			// E.g. A tall expression like 1^2 would have a higher top spacing than 3, so the util::max top spacing would be its top spacing;
			// So when drawing the 1^2, there is no difference between the util::max top spacing and the top spacing, and therefore it has
			// no top padding. But when drawing the 3, the difference between its top spacing and the util::max creates a top padding.
			if(ex->getType() == ObjType::CHAR_TYPE) {
				Character *ch = (Character*) ex;
				ch->draw(dest, x, y + (topSpacing - ch->getHeight() / 2));
				x += ch->getWidth() + EXPR_SPACING;
			}
			else {
				Expr *expr = (Expr*) ex;
				expr->draw(dest, x, y + (topSpacing - expr->topSpacing));
				// Increase x so nothing overlaps
				x += expr->exprWidth + EXPR_SPACING;
			}
		}
	}
	void Container::recomputeHeights() {
        // Do superscripts and subscripts first and then brackets
        // This is because the height of superscript and subscripts is a constant added to the height of the element before them
        // So if brackets were before them, this might cause an endless loop of increasing heights
		for(NEDAObj *ex : contents) {
			if(ex->getType() == ObjType::SUPERSCRIPT || ex->getType() == ObjType::SUBSCRIPT) {
				((Expr*) ex)->computeDimensions();
			}
		}
        for(NEDAObj *ex : contents) {
			if(ex->getType() == ObjType::L_BRACKET || ex->getType() == ObjType::R_BRACKET) {
				((Expr*) ex)->computeDimensions();
			}
		}
	}
    void Container::_add(NEDAObj *obj) {
        if(obj->getType() != ObjType::CHAR_TYPE) {
            static_cast<Expr*>(obj)->parent = this;
        }
        contents.add(obj);
    }
	void Container::add(NEDAObj *expr) {
		_add(expr);

		recomputeHeights();
        computeDimensions();
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
		computeDimensions();
		return obj;
	}
	void Container::addAt(uint16_t index, NEDAObj *exprToAdd) {
		contents.insert(exprToAdd, index);
		if(exprToAdd->getType() != ObjType::CHAR_TYPE) {
			((Expr*) exprToAdd)->parent = this;
		}

		recomputeHeights();
		computeDimensions();
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
			out.height = ((Character*) ref)->getHeight();
			out.y = y - ((Character*) ref)->getHeight() / 2 + topSpacing;
		}
		else {
			out.height = ((Expr*) ref)->exprHeight;
			out.y = y + topSpacing - ((Expr*) ref)->topSpacing;
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
    void Container::_addAtCursor(NEDAObj *obj, Cursor &cursor) {
        contents.insert(obj, cursor.index);
        ++cursor.index;
        if(obj->getType() != ObjType::CHAR_TYPE) {
            static_cast<Expr*>(obj)->parent = this;
        }
    }
	void Container::addAtCursor(NEDAObj *expr, Cursor &cursor) {
		_addAtCursor(expr, cursor);
        
		recomputeHeights();
		computeDimensions();
	}
	// Returns the expression removed for deletion
	NEDAObj* Container::removeAtCursor(Cursor &cursor) {
		if(cursor.index != 0) {
			NEDAObj *obj = contents[--cursor.index];
			contents.removeAt(cursor.index);
			recomputeHeights();
			computeDimensions();
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
			c->_add(ex->copy());
		}
        c->recomputeHeights();
        c->computeDimensions();
		
		return c;
	}
	void Container::addString(const char *str) {
		while(*str != '\0') {
			_add(new Character(*(str++)));
		}
        recomputeHeights();
        computeDimensions();
	}

	// *************************** Fraction ***************************************
    void Fraction::computeDimensions() {
        // The top spacing of a fraction is equal to the height of its numerator, plus a pixel of spacing between the numerator and
		// the fraction line.
        topSpacing = SAFE_ACCESS_0(numerator, exprHeight) + 1;
        // Take the greater of the widths and add 2 for the spacing at the sides
		uint16_t numeratorWidth = SAFE_ACCESS_0(numerator, exprWidth);
		uint16_t denominatorWidth = SAFE_ACCESS_0(denominator, exprWidth);
		exprWidth = util::max(numeratorWidth, denominatorWidth) + 2;
		// Take the sum of the heights and add 3 for the fraction line
        uint16_t numeratorHeight = SAFE_ACCESS_0(numerator, exprHeight);
		uint16_t denominatorHeight = SAFE_ACCESS_0(denominator, exprHeight);
		exprHeight = numeratorHeight + denominatorHeight + 3;
		
        SAFE_EXEC(parent, computeDimensions);
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
		computeDimensions();
	}
	void Fraction::setDenominator(Expr *denominator) {
		this->denominator = denominator;
		denominator->parent = this;
		computeDimensions();
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
    void LeftBracket::computeDimensions() {
        // Constant width
        exprWidth = 3;

        // There must be a parent container
        ASSERT_NONNULL(parent);
        if(parent->getType() != ObjType::CONTAINER) {
            return;
        }

        Container *parentContainer = (Container*) parent;
		auto &parentContents = parentContainer->contents;
		uint16_t index = parentContainer->indexOf(this);

		// The height and top spacing of the bracket depends on its content
		uint16_t nesting = 1;
        topSpacing = 0;
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
					auto charData = static_cast<Character*>(ex)->getCharData();

                    topSpacing = util::max(topSpacing, static_cast<uint16_t>(charData.height / 2));
				}
				else {
                    Expr *expr = static_cast<Expr*>(ex);

                    topSpacing = util::max(topSpacing, static_cast<uint16_t>(SAFE_ACCESS_0(expr, topSpacing)));
				}
			}
		}

        // If there's nothing after it, go with a default value
        if(topSpacing == 0) {
            topSpacing = Container::EMPTY_EXPR_HEIGHT / 2;
        }

        // Compute height separately as it requires the top spacing to be known
        // The logic here is the same as in Container
		nesting = 1;
        exprHeight = 0;
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
                    auto charData = static_cast<Character*>(ex)->getCharData();
                    
                    exprHeight = util::max(exprHeight, static_cast<uint16_t>(charData.height - charData.height / 2 + topSpacing));
				}
				else {
                    Expr *expr = static_cast<Expr*>(ex);

                    exprHeight = util::max(exprHeight, static_cast<uint16_t>(SAFE_ACCESS_0(expr, exprHeight) - SAFE_ACCESS_0(expr, topSpacing) + topSpacing));
				}
			}
		}

        // If there's nothing after it, go with a default value
        if(exprHeight == 0) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
        }

        // Notice how this method does not call computeDimensions() on its parent.
        // This is because the only time computeDimensions() will be called on a bracket is through
        // neda::Container's recomputeHeights() method, which is always followed by a call to 
        // computeDimensions() anyways. Calling computeDimensions() on its parent here will only
        // cause unnecessary calls.
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
    void LeftBracket::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
    }
	LeftBracket* LeftBracket::copy() {
		return new LeftBracket();
	}

	// *************************** RightBracket ***************************************
    void RightBracket::computeDimensions() {
        // Constant width
        exprWidth = 3;

        // There must be a parent container
        ASSERT_NONNULL(parent);
        if(parent->getType() != ObjType::CONTAINER) {
            return;
        }

        Container *parentContainer = (Container*) parent;
		auto &parentContents = parentContainer->contents;
		uint16_t index = parentContainer->indexOf(this);

		// The height and top spacing of the bracket depends on its content
		uint16_t nesting = 1;
        topSpacing = 0;
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
					auto charData = static_cast<Character*>(ex)->getCharData();

                    topSpacing = util::max(topSpacing, static_cast<uint16_t>(charData.height / 2));
				}
				else {
                    Expr *expr = static_cast<Expr*>(ex);

                    topSpacing = util::max(topSpacing, static_cast<uint16_t>(SAFE_ACCESS_0(expr, topSpacing)));
				}
			}
		}

        // If there's nothing before it, go with a default value
        if(topSpacing == 0) {
            topSpacing = Container::EMPTY_EXPR_HEIGHT / 2;
        }

        // Compute height separately as it requires the top spacing to be known
        // The logic here is the same as in Container
		nesting = 1;
        exprHeight = 0;
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
                    auto charData = static_cast<Character*>(ex)->getCharData();
                    
                    exprHeight = util::max(exprHeight, static_cast<uint16_t>(charData.height - charData.height / 2 + topSpacing));
				}
				else {
                    Expr *expr = static_cast<Expr*>(ex);

                    exprHeight = util::max(exprHeight, static_cast<uint16_t>(SAFE_ACCESS_0(expr, exprHeight) - SAFE_ACCESS_0(expr, topSpacing) + topSpacing));
				}
			}
		}

        // If there's nothing before it, go with a default value
        if(exprHeight == 0) {
            exprHeight = Container::EMPTY_EXPR_HEIGHT;
        }

        // Notice how this method does not call computeDimensions() on its parent.
        // This is because the only time computeDimensions() will be called on a bracket is through
        // neda::Container's recomputeHeights() method, which is always followed by a call to 
        // computeDimensions() anyways. Calling computeDimensions() on its parent here will only
        // cause unnecessary calls.
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
    void RightBracket::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;
    }
	RightBracket* RightBracket::copy() {
		return new RightBracket();
	}
	
	// *************************** Radical ***************************************
    void Radical::computeDimensions() {
        // No base
        if(!n) {
            if(contents) {
                // Add a few constants for the radical sign
                topSpacing = contents->topSpacing + 2;
                exprWidth = contents->exprWidth + 8;
                exprHeight = contents->exprHeight + 2;
            }
            else {
                topSpacing = exprHeight = exprWidth = 0;
            }
        }
        else {
            // Additional height added by the base of the radical.
            // It's either the height of the base minus the overlap or 0 in the case that the base is short.
            uint16_t additionalHeight = util::max(0, n->exprHeight - CONTENTS_N_OVERLAP);
            topSpacing = SAFE_ACCESS_0(contents, topSpacing) + 2 + additionalHeight;
            exprHeight = SAFE_ACCESS_0(contents, exprHeight) + 2 + additionalHeight;
            // The width is also the contents width plus the width of the base
            exprWidth =  SAFE_ACCESS_0(contents, exprWidth) + 8 + util::max(0, n->exprWidth - SIGN_N_OVERLAP);
        }

        SAFE_EXEC(parent, computeDimensions);
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
			uint16_t xoffset = util::max(0, n->exprWidth - SIGN_N_OVERLAP);
			uint16_t yoffset = util::max(0, n->exprHeight - CONTENTS_N_OVERLAP);
			dest.drawLine(x + xoffset, y + exprHeight - 1 - 2, x + 2 + xoffset, y + exprHeight - 1);
			dest.drawLine(x + 2 + xoffset, y + exprHeight - 1, x + 6 + xoffset, y + yoffset);
			dest.drawLine(x + 6 + xoffset, y + yoffset, x + exprWidth - 1, y + yoffset);
			
			contents->draw(dest, x + 7 + xoffset, y + 2 + yoffset);
		}
	}
	void Radical::setContents(Expr *contents) {
		this->contents = contents;
		contents->parent = this;
		computeDimensions();
	}
	void Radical::setN(Expr *n) {
		this->n = n;
		n->parent = this;
		computeDimensions();
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
    void Superscript::computeDimensions() {
        // Width is the same as the contents
        exprWidth = SAFE_ACCESS_0(contents, exprWidth);
        // There must be a parent container
        ASSERT_NONNULL(parent);
        if(parent->getType() != ObjType::CONTAINER) {
            return;
        }

        Container *parentContainer = (Container*) parent;
		auto &parentContents = parentContainer->contents;
		uint16_t index = parentContainer->indexOf(this);
		// Look at the expression right before it. If there is no expression before, go with the default
		if(index == 0) {
			topSpacing = SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT / 2 - OVERLAP;
            exprHeight = SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT - OVERLAP;
		}
        else {
            NEDAObj *prevObj = parentContents[index - 1];
            if(prevObj->getType() == ObjType::CHAR_TYPE) {
                auto charData = static_cast<Character*>(prevObj)->getCharData();

                topSpacing = charData.height / 2 + SAFE_ACCESS_0(contents, exprHeight) - OVERLAP;
                exprHeight = charData.height + SAFE_ACCESS_0(contents, exprHeight) - OVERLAP;
            }
            else {
                Expr *ex = static_cast<Expr*>(prevObj);

                topSpacing = SAFE_ACCESS_0(ex, topSpacing) + SAFE_ACCESS_0(contents, exprHeight) - OVERLAP;
                exprHeight = SAFE_ACCESS_0(ex, exprHeight) + SAFE_ACCESS_0(contents, exprHeight) - OVERLAP;
            }
        }

        SAFE_EXEC(parent, computeDimensions);
    }
	void Superscript::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
		this->x = x;
		this->y = y;
		VERIFY_INBOUNDS(x, y);
		SAFE_EXEC(contents, draw, dest, x, y);
	}
	void Superscript::setContents(Expr *contents) {
		this->contents = contents;
		computeDimensions();
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
    void Subscript::computeDimensions() {
        // Width is the same as the contents
        exprWidth = SAFE_ACCESS_0(contents, exprWidth);
        // There must be a parent container
        ASSERT_NONNULL(parent);
        if(parent->getType() != ObjType::CONTAINER) {
            return;
        }

        Container *parentContainer = (Container*) parent;
		auto &parentContents = parentContainer->contents;
		uint16_t index = parentContainer->indexOf(this);
		// Look at the expression right before it. If there is no expression before, go with the default
		if(index == 0) {
			topSpacing = SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT / 2 - OVERLAP;
            exprHeight = SAFE_ACCESS_0(contents, exprHeight) + Container::EMPTY_EXPR_HEIGHT - OVERLAP;
		}
        else {
            NEDAObj *prevObj = parentContents[index - 1];
            if(prevObj->getType() == ObjType::CHAR_TYPE) {
                auto charData = static_cast<Character*>(prevObj)->getCharData();

                // Copy the top spacing of the previous element
                topSpacing = charData.height / 2;
                exprHeight = charData.height + SAFE_ACCESS_0(contents, exprHeight) - OVERLAP;
            }
            else {
                Expr *ex = static_cast<Expr*>(prevObj);

                // Copy the top spacing of the previous element
                topSpacing = SAFE_ACCESS_0(ex, topSpacing);
                exprHeight = SAFE_ACCESS_0(ex, exprHeight) + SAFE_ACCESS_0(contents, exprHeight) - OVERLAP;
            }
        }

        SAFE_EXEC(parent, computeDimensions);
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
		computeDimensions();
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
    void SigmaPi::computeDimensions() {
        // The top spacing of this expr can be split into two cases: when the contents are tall and when the contents are short.
		// When the contents are tall enough, the result is simply the top spacing of the contents (b)
		// Otherwise, it is the distance from the top to the middle of the base of the contents.
		// Therefore, we add up the heights of the expr at the top, the spacing, and the overlap between the symbol and the contents,
		// then subtract half of the base height of the contents.
        // However, since there might be subscripts involved, taking height - top spacing doesn't always equal half of the base
        // height. But as all characters currently have uniform height, we can use the makeshift solution of just using half of the
        // empty container height (which is equal to the uniform character height).
		uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
				- (Container::EMPTY_EXPR_HEIGHT / 2);
		uint16_t b = SAFE_ACCESS_0(contents, topSpacing);
		topSpacing = util::max(a, b);

        // The width is split into two parts: the width of the conditions and symbol, and the width of the contents.
        // The first part is just the maximum of the three.
        uint16_t topWidth = SAFE_ACCESS_0(finish, exprWidth);
		uint16_t bottomWidth = SAFE_ACCESS_0(start, exprWidth);
		exprWidth = util::max(symbol.width, util::max(topWidth, bottomWidth)) + 3 + SAFE_ACCESS_0(contents, exprWidth);

        // Logic here is similar to that of Container
        uint16_t symbolHeight = SAFE_ACCESS_0(finish, exprHeight) + 2 + symbol.height + 2 + SAFE_ACCESS_0(start, exprHeight)
				+ topSpacing - a;
		uint16_t bodyHeight = SAFE_ACCESS_0(contents, exprHeight) + topSpacing - b;
		exprHeight = util::max(symbolHeight, bodyHeight);

        SAFE_EXEC(parent, computeDimensions);
    }
	void SigmaPi::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
		this->x = x;
		this->y = y;
		VERIFY_INBOUNDS(x, y);
		ASSERT_NONNULL(start);
		ASSERT_NONNULL(finish);
		ASSERT_NONNULL(contents);
		
		// Top spacings - Taken from neda::SigmaPi::computeDimensions()
		uint16_t a = SAFE_ACCESS_0(finish, exprHeight) + 2 + CONTENT_SYMBOL_OVERLAP 
				- (Container::EMPTY_EXPR_HEIGHT / 2);
		uint16_t b = SAFE_ACCESS_0(contents, topSpacing);

		// Logic same as neda::Container::draw()
		uint16_t symbolYOffset = topSpacing - a;

		uint16_t contentsYOffset = topSpacing - b;

		// Center the top, the bottom and the symbol
		uint16_t widest = util::max(start->exprWidth, util::max(finish->exprWidth, symbol.width));
		finish->draw(dest, x + (widest - finish->exprWidth) / 2, y + symbolYOffset);
		dest.drawImage(x + (widest - symbol.width) / 2, y + finish->exprHeight + 2 + symbolYOffset, symbol);
		start->draw(dest, x + (widest - start->exprWidth) / 2, y + finish->exprHeight + 2 + symbol.height + 2 + symbolYOffset);

		contents->draw(dest, x + widest + 3, y + contentsYOffset);
	}
	void SigmaPi::setStart(Expr *start) {
		this->start = start;
		start->parent = this;
		
        computeDimensions();
	}
	void SigmaPi::setFinish(Expr *finish) {
		this->finish = finish;
		finish->parent = this;
		
        computeDimensions();
	}
	void SigmaPi::setContents(Expr *contents) {
		this->contents = contents;
		contents->parent = this;
		
        computeDimensions();
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

	// *************************** Matrix ***************************************
	ObjType Matrix::getType() const {
		return ObjType::MATRIX;
	}
	Matrix::~Matrix() {
		for(uint16_t i = 0; i < m * n; i ++) {
			DESTROY_IF_NONNULL(contents[i]);
		}
		delete[] contents;
    }
	uint16_t Matrix::rowTopSpacing_0(uint8_t row) {
		uint16_t rowMax = 0;
		for(uint8_t j = 0; j < n; j ++) {
			rowMax = util::max(rowMax, static_cast<uint16_t>(SAFE_ACCESS_0(contents[index_0(j, row)], topSpacing)));
		}
		return rowMax;
	}
	uint16_t Matrix::rowHeight_0(uint8_t row) {
		uint16_t rowMax = 0;
		for(uint8_t j = 0; j < n; j ++) {
			rowMax = util::max(rowMax, static_cast<uint16_t>(SAFE_ACCESS_0(contents[index_0(j, row)], exprHeight)));
		}
		return rowMax;
	}
	uint16_t Matrix::colWidth_0(uint8_t col) {
		uint16_t colMax = 0;
		for(uint8_t j = 0; j < m; j ++) {
			colMax = util::max(colMax, static_cast<uint16_t>(SAFE_ACCESS_0(contents[index_0(col, j)], exprWidth)));
		}
		return colMax;
	}
    void Matrix::computeDimensions() {
        // Go through every row in the top half
		topSpacing = 0;
		for(uint8_t i = 0; i < m / 2; i ++) {
			topSpacing += rowHeight_0(i);
		}
		// Add edge spacing (top)
		topSpacing += TOP_SPACING;
		
		// The top spacing is divided into two scenarios
		// Scenario 1: even number of rows
		if(m % 2 == 0) {
			// Add spacing between rows
			topSpacing += (m / 2 - 1) * ENTRY_SPACING;
			// Add spacing between middle two rows
			topSpacing += ENTRY_SPACING / 2;
		}
		// Scenario 2: odd number of rows
		else {
			// Add spacing between rows
			topSpacing += m / 2 * ENTRY_SPACING;
			// Add the top spacing of the middle row
			topSpacing += rowTopSpacing_0(m / 2);
		}
        
        exprHeight = 0;
		for(uint8_t i = 0; i < m; i ++) {
			// For every row find the util::max height
			exprHeight += rowHeight_0(i);
		}
		exprHeight += (m - 1) * ENTRY_SPACING;
		exprHeight += 2 * TOP_SPACING;

        exprWidth = 0;
		for(uint8_t i = 0; i < n; i ++) {
			exprWidth += colWidth_0(i);
		}
		exprWidth += (n - 1) * ENTRY_SPACING;
		exprWidth += 2 * SIDE_SPACING;

        SAFE_EXEC(parent, computeDimensions);
    }
	void Matrix::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
		this->x = x;
		this->y = y;
		VERIFY_INBOUNDS(x, y);

		// Draw contents first
		// Cache column widths
		uint16_t *colWidths = new uint16_t[n];
		for(uint8_t i = 0; i < n; i ++) {
			colWidths[i] = colWidth_0(i);
		}

		// Go row-by-row
		uint16_t exprY = y + TOP_SPACING;
		for(uint8_t row = 0; row < m; row ++) {
			uint16_t exprX = x + SIDE_SPACING;
			uint16_t topSpacing = rowTopSpacing_0(row);
			
			for(uint8_t col = 0; col < n; col ++) {
                if(contents) {
                    uint16_t index = index_0(col, row);
                    // Centre the expression
                    uint16_t offset = (colWidths[col] - contents[index]->exprWidth) / 2;
                    contents[index]->draw(dest, exprX + offset, exprY + (topSpacing - contents[index]->topSpacing));
                }
				exprX += colWidths[col] + ENTRY_SPACING;
			}
			
			exprY += rowHeight_0(row) + ENTRY_SPACING;
		}

		// Draw square brackets
		for(uint16_t i = 0; i < exprHeight; i ++) {
			dest.setPixel(x, y + i, true);
			dest.setPixel(x + exprWidth - 1, y + i, true);
		}
		dest.setPixel(x + 1, y, true);
		dest.setPixel(x + 1, y + exprHeight - 1, true);
		dest.setPixel(x + exprWidth - 2, y, true);
		dest.setPixel(x + exprWidth - 2, y + exprHeight - 1, true);

		delete[] colWidths;
	}
	void Matrix::getCursor(Cursor &cursor, CursorLocation location) {
		if(location == CURSORLOCATION_START) {
			SAFE_EXEC(contents[0], getCursor, cursor, location);
		}
		else {
			SAFE_EXEC(contents[n - 1], getCursor, cursor, location);
		}
	}
	void Matrix::left(Expr *ex, Cursor &cursor) {
		// Find the index
		uint8_t x, y;
		for(x = 0; x < n; x ++) {
			for(y = 0; y < m; y ++) {
				if(ex == contents[index_0(x, y)]) {
					goto loopEnd;
				}
			}
		}
loopEnd:
		if(x != 0 && x != n) {
			SAFE_EXEC(contents[index_0(x - 1, y)], getCursor, cursor, CURSORLOCATION_END);
		}
		else {
			SAFE_EXEC(parent, left, this, cursor);
		}
	}
	void Matrix::right(Expr *ex, Cursor &cursor) {
		// Find the index
		uint8_t x, y;
		for(x = 0; x < n; x ++) {
			for(y = 0; y < m; y ++) {
				if(ex == contents[index_0(x, y)]) {
					goto loopEnd;
				}
			}
		}
loopEnd:
		if(x < n - 1) {
			SAFE_EXEC(contents[index_0(x + 1, y)], getCursor, cursor, CURSORLOCATION_START);
		}
		else {
			SAFE_EXEC(parent, right, this, cursor);
		}
	}
	void Matrix::up(Expr *ex, Cursor &cursor) {
		// Find the index
		uint8_t x, y;
		for(x = 0; x < n; x ++) {
			for(y = 0; y < m; y ++) {
				if(ex == contents[index_0(x, y)]) {
					goto loopEnd;
				}
			}
		}
loopEnd:
		if(y != 0 && y != m) {
			SAFE_EXEC(contents[index_0(x, y - 1)], getCursor, cursor, CURSORLOCATION_END);
		}
		else {
			SAFE_EXEC(parent, up, this, cursor);
		}
	}
	void Matrix::down(Expr *ex, Cursor &cursor) {
		// Find the index
		uint8_t x, y;
		for(x = 0; x < n; x ++) {
			for(y = 0; y < m; y ++) {
				if(ex == contents[index_0(x, y)]) {
					goto loopEnd;
				}
			}
		}
loopEnd:
		if(y < m - 1) {
			SAFE_EXEC(contents[index_0(x, y + 1)], getCursor, cursor, CURSORLOCATION_START);
		}
		else {
			SAFE_EXEC(parent, down, this, cursor);
		}
	}
    void Matrix::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;

        for(uint16_t i = 0; i < m * n; i ++) {
            SAFE_EXEC(contents[i], updatePosition, dx, dy);
        }
    }
	Matrix* Matrix::copy() {
		Matrix *mat = new Matrix(m, n);
		
		for(uint16_t i = 0; i < m * n; i ++) {
			mat->contents[i] = (neda::Expr*) contents[i]->copy();
			mat->contents[i]->parent = mat;
		}
		mat->exprWidth = exprWidth;
		mat->exprHeight = exprHeight;
        mat->topSpacing = topSpacing;
		return mat;
	}

    // *************************** Piecewise ***************************************
    ObjType Piecewise::getType() const {
        return ObjType::PIECEWISE;
    }
    Piecewise::~Piecewise() {
        for(uint8_t i = 0; i < pieces; i ++) {
            DESTROY_IF_NONNULL(values[i]);
            DESTROY_IF_NONNULL(conditions[i]);
        }
        delete[] values;
        delete[] conditions;
    }
    void Piecewise::computeDimensions() {
        // Go through every row in the top half
		topSpacing = 0;
		for(uint8_t i = 0; i < pieces / 2; i ++) {
			topSpacing += util::max(SAFE_ACCESS_0(values[i], exprHeight), SAFE_ACCESS_0(conditions[i], exprHeight));
		}
		// Add edge spacing (top)
		topSpacing += TOP_SPACING;
		
		// The top spacing is divided into two scenarios
		// Scenario 1: even number of rows
		if(pieces % 2 == 0) {
			// Add spacing between rows
			topSpacing += (pieces / 2 - 1) * ROW_SPACING;
			// Add spacing between middle two rows
			topSpacing += ROW_SPACING / 2;
		}
		// Scenario 2: odd number of rows
		else {
			// Add spacing between rows
			topSpacing += pieces / 2 * ROW_SPACING;
			// Add the top spacing of the middle row
			topSpacing += util::max(SAFE_ACCESS_0(values[pieces / 2], topSpacing), SAFE_ACCESS_0(conditions[pieces / 2], topSpacing));
		}

        uint16_t widestValue = 0;
        uint16_t widestCondition = 0;
        for(uint8_t i = 0; i < pieces; i ++) {
            widestValue = util::max(widestValue, static_cast<uint16_t>(SAFE_ACCESS_0(values[i], exprWidth)));
            widestCondition = util::max(widestCondition, static_cast<uint16_t>(SAFE_ACCESS_0(conditions[i], exprWidth)));
        }
        exprWidth = widestValue + widestCondition + LEFT_SPACING + VALUE_CONDITION_SPACING;

        exprHeight = TOP_SPACING + TOP_SPACING + (pieces - 1) * ROW_SPACING;
        for(uint8_t i = 0; i < pieces; i ++) {
            exprHeight += util::max(SAFE_ACCESS_0(values[i], exprHeight), SAFE_ACCESS_0(conditions[i], exprHeight));
        }

        SAFE_EXEC(parent, computeDimensions);
    }
    void Piecewise::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
		this->y = y;
		VERIFY_INBOUNDS(x, y);

		// Draw contents first
		uint16_t maxValueWidth = 0;
        for(uint8_t i = 0; i < pieces; i ++) {
            maxValueWidth = util::max(maxValueWidth, static_cast<uint16_t>(SAFE_ACCESS_0(values[i], exprWidth)));
        }

		// Go row-by-row
		uint16_t exprY = y + TOP_SPACING;
		for(uint16_t i = 0; i < pieces; i ++) {
            if(values[i] && conditions[i]) {
                uint16_t valueTopSpacing = values[i]->topSpacing;
                uint16_t conditionTopSpacing = conditions[i]->topSpacing;
                uint16_t maxTopSpacing = util::max(valueTopSpacing, conditionTopSpacing);

                values[i]->draw(dest, x + LEFT_SPACING, exprY + (maxTopSpacing - valueTopSpacing));
                conditions[i]->draw(dest, x + LEFT_SPACING + maxValueWidth + VALUE_CONDITION_SPACING, exprY + (maxTopSpacing - conditionTopSpacing));

                exprY += util::max(values[i]->exprHeight, conditions[i]->exprHeight) + ROW_SPACING;
            }
		}

        // Draw curly bracket
        // Top curl
        dest.setPixel(x + 2, y, true);
        // Bottom curl
        dest.setPixel(x + 2, y + exprHeight - 1, true);
        // Middle
        dest.setPixel(x, y + (exprHeight / 2) - 1, true);
        // Main body
        for(int16_t py = y + 1; py < y + exprHeight - 1; py ++) {
            if(py != y + (exprHeight / 2) - 1) {
                dest.setPixel(x + 1, py, true);
            }
        }
    }
    void Piecewise::getCursor(Cursor &cursor, CursorLocation location) {
        if(location == CURSORLOCATION_START) {
            SAFE_EXEC(values[0], getCursor, cursor, location);
        }
        else {
            SAFE_EXEC(conditions[0], getCursor, cursor, location);
        }
    }
    void Piecewise::left(Expr *expr, Cursor &cursor) {
        // Find the expression the cursor is in
        bool isCondition = false;
        uint8_t i = 0;
        for(; i < pieces; i ++) {
            if(expr == values[i]) {
                break;
            }
            else if(expr == conditions[i]) {
                isCondition = true;
                break;
            }
        }

        // If cursor is in the condition, move it to the value
        if(isCondition) {
            SAFE_EXEC(values[i], getCursor, cursor, CURSORLOCATION_END);
        }
        else {
            SAFE_EXEC(parent, left, this, cursor);
        }
    }
    void Piecewise::right(Expr *expr, Cursor &cursor) {
        // Find the expression the cursor is in
        bool isCondition = false;
        uint8_t i = 0;
        for(; i < pieces; i ++) {
            if(expr == values[i]) {
                break;
            }
            else if(expr == conditions[i]) {
                isCondition = true;
                break;
            }
        }
        
        if(!isCondition) {
            SAFE_EXEC(conditions[i], getCursor, cursor, CURSORLOCATION_START);
        }
        else {
            SAFE_EXEC(parent, right, this, cursor);
        }
    }
    void Piecewise::up(Expr *expr, Cursor &cursor) {
        // Find the expression the cursor is in
        bool isCondition = false;
        uint8_t i = 0;
        for(; i < pieces; i ++) {
            if(expr == values[i]) {
                break;
            }
            else if(expr == conditions[i]) {
                isCondition = true;
                break;
            }
        }

        if(i == 0) {
            SAFE_EXEC(parent, up, this, cursor);
        }
        else {
            SAFE_EXEC(isCondition ? conditions[i - 1] : values[i - 1], getCursor, cursor, CURSORLOCATION_END);
        }
    }
    void Piecewise::down(Expr *expr, Cursor &cursor) {
        // Find the expression the cursor is in
        bool isCondition = false;
        uint8_t i = 0;
        for(; i < pieces; i ++) {
            if(expr == values[i]) {
                break;
            }
            else if(expr == conditions[i]) {
                isCondition = true;
                break;
            }
        }

        if(i == pieces - 1) {
            SAFE_EXEC(parent, down, this, cursor);
        }
        else {
            SAFE_EXEC(isCondition ? conditions[i + 1] : values[i + 1], getCursor, cursor, CURSORLOCATION_START);
        }
    }
    void Piecewise::updatePosition(int16_t dx, int16_t dy) {
        this->x += dx;
        this->y += dy;

        for(uint8_t i = 0; i < pieces; i ++) {
            SAFE_EXEC(values[i], updatePosition, dx, dy);
            SAFE_EXEC(conditions[i], updatePosition, dx, dy);
        }
    }
    Piecewise* Piecewise::copy() {
        Piecewise *other = new Piecewise(pieces);
        for(uint8_t i = 0; i < pieces; i ++) {
            other->values[i] = static_cast<neda::Expr*>(values[i]->copy());
            other->conditions[i] = static_cast<neda::Expr*>(conditions[i]->copy());
            other->values[i]->parent = other;
            other->conditions[i]->parent = other;
        }
        other->exprWidth = exprWidth;
        other->exprHeight = exprHeight;
        other->topSpacing = topSpacing;
        return other;
    }

    // *************************** Abs ***************************************
    ObjType Abs::getType() const {
        return ObjType::ABS;
    }
    Abs::~Abs() {
        DESTROY_IF_NONNULL(contents);
    }
    void Abs::computeDimensions() {

        topSpacing = SAFE_ACCESS_0(contents, topSpacing) + 1;
        exprWidth = SAFE_ACCESS_0(contents, exprWidth) + 4;
        exprHeight = SAFE_ACCESS_0(contents, exprHeight) + 2;

        SAFE_EXEC(parent, computeDimensions);
    }
    void Abs::draw(lcd::LCD12864 &dest, int16_t x, int16_t y) {
        this->x = x;
        this->y = y;
        VERIFY_INBOUNDS(x, y);

        // Draw contents
        SAFE_EXEC(contents, draw, dest, x + 2, y + 1);

        // Draw vertical bars
        for(uint16_t i = 0; i < exprHeight; i ++) {
            dest.setPixel(x, y + i, true);
            dest.setPixel(x + exprWidth - 1, y + i, true);
        }
    }
    void Abs::getCursor(Cursor &cursor, CursorLocation location) {
        SAFE_EXEC(contents, getCursor, cursor, location);
    }
    void Abs::updatePosition(int16_t dx, int16_t dy) {
        x += dx;
        y += dy;
        
        if(contents) {
            contents->x += dx;
            contents->y += dy;
        }
    }
    Abs* Abs::copy() {
        if(contents) {
            return new Abs(static_cast<neda::Expr*>(contents->copy()));
        }
        else {
            return new Abs;
        }
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
    void Cursor::addStr(const char *str) {
        while(*str != '\0') {
            expr->_addAtCursor(new neda::Character(*str++), *this);
        }
        expr->recomputeHeights();
        expr->computeDimensions();
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
