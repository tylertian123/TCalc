#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"
#include "dynamarr.hpp"
#include <string.h>

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

    class Cursor;
    // Note: CursorInfo is the specific position of the cursor on the display, with x and y coordinates
    // CursorLocation can either be START or END and is used to get a cursor at the start or end of the expr.
    struct CursorInfo;
    typedef bool CursorLocation;
    constexpr CursorLocation CURSORLOCATION_START = 0;
    constexpr CursorLocation CURSORLOCATION_END = 1;

    // Enum of all NEDA object types
    enum class ObjType : uint8_t {
        NULL_TYPE,
        CHAR_TYPE,
        CURSOR_TYPE,
        STRING,
        CONTAINER,
        FRACTION,
        L_BRACKET,
        R_BRACKET,
        RADICAL,
        SUPERSCRIPT,
        SUBSCRIPT,
        SIGMA_PI,
        MATRIX,
    };
	
    /*
     * This is the base class all NEDA classes inherit from. 
     */ 
    class NEDAObj {
    public:
        virtual ObjType getType() = 0;

        virtual NEDAObj* copy() = 0;

        virtual ~NEDAObj() {}
    };

    /*
     * This class represents a single character. It only inherits from NEDAObj and not Expr in order to save memory.
     */
    class Character : public NEDAObj {
    public:
        Character(const char ch) : ch(ch) {}

        virtual ObjType getType() override {
            return ObjType::CHAR_TYPE;
        }

        const char ch;
        void draw(lcd::LCD12864 &lcd, int16_t, int16_t);
        uint16_t getWidth();
        uint16_t getHeight();

        virtual Character* copy() override;
    };

	/*
     * This is the base Expression class.
     * 
     * Every NEDA object is made of nested expressions, hence the name.
     * Every expression in NEDA has 3 properties: a width, a height, and a top spacing, and can be drawn.
     * The top spacing is so that expressions in a group line up nicely, e.g. in the expression 1^2+3, the 1, plus sign and 3 should
     * line up, despite 1^2 being taller than the other expressions. 
     * The top spacing refers to the distance between the very top of the expression to the middle of the "base" part of the 
     * expression (rounded down). For example, the top spacing of 1^2 would be the distance in pixels from the top of the 2 to the
     * middle of the 1, and the top spacing of 1 would just be half the height of 1.
     * The top spacing is to the middle of the base expression to accommodate for things with different heights such as fractions.
     */
	class Expr : public NEDAObj {
	public:
        // The width, height, x and y coordinates are all cached
		virtual void computeWidth() = 0;
		virtual void computeHeight() = 0;

        virtual void updatePosition(int16_t, int16_t);
	
		virtual uint16_t getTopSpacing() = 0;
	
        // Draws the expr at the specified coords, updating the cached x and y as it goes
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) = 0;
        // Draws the expr at the cached coords
        void draw(lcd::LCD12864&);
        // Draws all expressions that are connected in some way to this one. e.g. its parents, siblings, grandparents, etc.
        void drawConnected(lcd::LCD12864&);
        // Gets the one top-level expr, a direct parent of this expr that has no parent
        Expr* getTopLevel();
	
		virtual ~Expr() {};

        Expr *parent = nullptr;
        virtual void left(Expr*, Cursor&);
        virtual void right(Expr*, Cursor&);
        virtual void up(Expr*, Cursor&);
        virtual void down(Expr*, Cursor&);
        virtual void getCursor(Cursor&, CursorLocation) = 0;

        virtual ObjType getType() = 0;
	
		uint16_t exprWidth;
		uint16_t exprHeight;
        int16_t x;
        int16_t y;
	};
	
	/*
     * The Container is an expression that serves as a container for a bunch of other expressions.
     * Containers have special logic in their drawing code that make sure everything lines up using the top spacing.
     */
	class Container : public Expr {
	public:
		// Constructor from dynamic array of Expression pointers, copy constructor and default constructor
		Container(const DynamicArray<NEDAObj*> &exprs) : contents(exprs) {
            for(NEDAObj* ex : contents) {
                if(ex->getType() != ObjType::CHAR_TYPE) {
                    ((Expr*)ex)->parent = this;
                }
            }
			computeWidth();
			computeHeight();
		}
		Container(const Container &other) : contents(other.contents) {
            for(NEDAObj* ex : contents) {
                if(ex->getType() != ObjType::CHAR_TYPE) {
                    ((Expr*)ex)->parent = this;
                }
            }
			computeWidth();
			computeHeight();
		}
		Container() : contents() {
			computeWidth();
			computeHeight();
		}

        static constexpr uint16_t EMPTY_EXPR_WIDTH = 5;
        static constexpr uint16_t EMPTY_EXPR_HEIGHT = 9;
        static constexpr uint16_t EXPR_SPACING = 1;
		
		void add(NEDAObj*);
        NEDAObj* remove(uint16_t);
        void addAt(uint16_t, NEDAObj*);
        uint16_t indexOf(NEDAObj*);
			
        virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		virtual ~Container();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void up(Expr*, Cursor&) override;
        virtual void down(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        void drawCursor(lcd::LCD12864&, const Cursor&);
        void addAtCursor(NEDAObj*, Cursor&);
        NEDAObj* removeAtCursor(Cursor&);
        void getCursorInfo(const Cursor&, CursorInfo&);

        virtual ObjType getType() override {
            return ObjType::CONTAINER;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
		DynamicArray<NEDAObj*> contents;
        // Recomputes the heights of all expressions that have heights dependent on others
        void recomputeHeights();

        virtual Container* copy() override;

        void addString(const char*);
	};
	
	// Fraction
	class Fraction : public Expr {
	public:
		Fraction(Expr *numerator, Expr *denominator) : numerator(numerator), denominator(denominator) {
            numerator->parent = this;
            denominator->parent = this;
			computeWidth();
			computeHeight();
		}
		Fraction() : numerator(nullptr), denominator(nullptr) {
			computeWidth();
			computeHeight();
		}
		
        virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
			
		Expr* getNumerator();
		Expr* getDenominator();
		void setNumerator(Expr*);
		void setDenominator(Expr*);
		
		virtual ~Fraction();

        virtual void up(Expr*, Cursor&) override;
        virtual void down(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ObjType getType() override {
            return ObjType::FRACTION;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
		Expr *numerator;
		Expr *denominator;

        virtual Fraction* copy() override;
	};
	
	//
    class LeftBracket : public Expr {
    public:
        LeftBracket() {
            computeWidth();
            computeHeight();
        }

        virtual uint16_t getTopSpacing() override;
        virtual void computeWidth() override;
        virtual void computeHeight() override;
        virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
        // Do nothing
        // Realistically this method is never going to be called on LeftBracket anyways
        virtual void getCursor(Cursor &cursor, CursorLocation location) override {}
        virtual ObjType getType() override {
            return ObjType::L_BRACKET;
        }

        virtual LeftBracket* copy() override;
    };
    class RightBracket : public Expr {
    public:
        RightBracket() {
            computeWidth();
            computeHeight();
        }

        virtual uint16_t getTopSpacing() override;
        virtual void computeWidth() override;
        virtual void computeHeight() override;
        virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
        // Do nothing
        // Realistically this method is never going to be called on RightBracket anyways
        virtual void getCursor(Cursor &cursor, CursorLocation location) override {}
        virtual ObjType getType() override {
            return ObjType::R_BRACKET;
        }

        virtual RightBracket* copy() override;
    };

	
	// n-th root expression
	class Radical : public Expr {
	public:
		Radical(Expr *contents, Expr *n) : contents(contents), n(n) {
            if(contents) {
                contents->parent = this;
            }
            if(n) {
                n->parent = this;
            }
			computeWidth();
			computeHeight();
		}
		Radical() : contents(nullptr), n(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		static constexpr uint16_t CONTENTS_N_OVERLAP = 7;
		static constexpr uint16_t SIGN_N_OVERLAP = 1;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		void setContents(Expr*);
		void setN(Expr*);
		
		virtual ~Radical();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ObjType getType() override {
            return ObjType::RADICAL;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
		Expr *contents, *n;

        virtual Radical* copy() override;
	};

    //
    class Superscript : public Expr {
    public:
        Superscript(Expr *contents) : contents(contents) {
            contents->parent = this;
            computeWidth();
            computeHeight();
        }
        Superscript() : contents(nullptr) {
            computeWidth();
            computeHeight();
        }

        static constexpr uint16_t OVERLAP = 4;

        virtual uint16_t getTopSpacing() override;
        virtual void computeWidth() override;
        virtual void computeHeight() override;
        virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;

        void setContents(Expr*);

        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ~Superscript();

        virtual ObjType getType() override {
            return ObjType::SUPERSCRIPT;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        Expr *contents;

        virtual Superscript* copy() override;
    };

	// Subscript
	class Subscript : public Expr {
	public:
		Subscript(Expr *contents) : contents(contents) {
            contents->parent = this;
			computeWidth();
			computeHeight();
		}
		Subscript() : contents(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		static constexpr uint16_t OVERLAP = 4;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		void setContents(Expr*);
		
		virtual ~Subscript();

        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ObjType getType() override {
            return ObjType::SUBSCRIPT;
        }

        virtual void updatePosition(int16_t, int16_t) override;
		
		Expr *contents;

        virtual Subscript* copy() override;
	};
	
	// Summation (Sigma) or Product (Pi)
	class SigmaPi : public Expr {
	public:
		SigmaPi(const lcd::LCD12864Image &symbol, Expr *start, Expr *finish, Expr *contents) : symbol(symbol), start(start), finish(finish), contents(contents) {
			start->parent = this;
            finish->parent = this;
            contents->parent = this;
            computeWidth();
			computeHeight();
		}
		SigmaPi(const lcd::LCD12864Image &symbol) : symbol(symbol), start(nullptr), finish(nullptr), contents(nullptr) {
			computeWidth();
			computeHeight();
		}

        static constexpr uint16_t CONTENT_SYMBOL_OVERLAP = 12;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		void setStart(Expr *start);
		void setFinish(Expr *finish);
		void setContents(Expr *contents);
		
		virtual ~SigmaPi();

        virtual void right(Expr*, Cursor&) override;
        virtual void up(Expr*, Cursor&) override;
        virtual void down(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ObjType getType() override {
            return ObjType::SIGMA_PI;
        }

        virtual void updatePosition(int16_t, int16_t) override;
		
		const lcd::LCD12864Image &symbol;
		Expr *start, *finish, *contents;

        virtual SigmaPi* copy() override;
	};

    // Matrix/Column Vector
    class Matrix : public Expr {
    public:
        Matrix(uint8_t m, uint8_t n) : m(m), n(n) {
            // Allocate array and set all entries to null
            contents = new Expr*[m * n];
            memset(contents, NULL, m * n * sizeof(Expr*));
        }

        virtual ~Matrix();

        static constexpr uint16_t ENTRY_SPACING = 4;
        static constexpr uint16_t SIDE_SPACING = 3;
        static constexpr uint16_t TOP_SPACING = 2;

        // Rows
        const uint8_t m;
        // Cols
        const uint8_t n;
        Expr **contents;

        // Maps zero-based indexing to index in contents array
        inline uint16_t index_0(uint8_t x, uint8_t y) {
            return x + y * n;
        }
        // Sets an entry
        // DOES NOT RECOMPUTE THE SIZE!
        inline void setEntry(uint8_t row, uint8_t col, Expr *entry) {
            contents[index_0(row, col)] = entry;
            entry->parent = this;
        }
        inline Expr* getEntry(uint8_t row, uint8_t col) {
            return contents[index_0(row, col)];
        }

        // 0-based indexing!
        uint16_t rowTopSpacing_0(uint8_t row);
        uint16_t rowHeight_0(uint8_t row);
        uint16_t colWidth_0(uint8_t col);

        virtual uint16_t getTopSpacing() override;
        virtual void computeWidth() override;
        virtual void computeHeight() override;
        virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void up(Expr*, Cursor&) override;
        virtual void down(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual Matrix* copy() override;

        virtual ObjType getType() override;
    };
	
	/*
	 * This struct contains info about the cursor's position and size.
	 */
	struct CursorInfo {
        int16_t x;
        int16_t y;
        uint16_t width;
        uint16_t height;
    };
    /*
     * This struct represents the location of the cursor. 
     */
    class Cursor : public NEDAObj {
    public:
        Container *expr;
        uint16_t index;

        void draw(lcd::LCD12864& dest);
        void left();
        void right();
        void up();
        void down();
        void getInfo(CursorInfo &info);
        void add(NEDAObj*);

        virtual ObjType getType() override {
            return ObjType::CURSOR_TYPE;
        }
        
        virtual Cursor* copy() override {
            Cursor *c = new Cursor;
            c->expr = expr;
            c->index = index;
            return c;
        }
    };

    Container* makeString(const char*);
}

#endif
