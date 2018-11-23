#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"
#include "dynamarr.hpp"
#include <string.h>

//Nested Expression Display Algorithm
//WARNING: All instances have to be allocated on the heap with new
//If allocated on the stack the object must not go out of scope
//DO NOT ALLOCATE USING malloc
/*
 * NEDA: Nested Expression Display Algorithm
 * 
 * WARNING: To prevent possible memory leaks, all NEDA classes have destructors that deletes all its children.
 * This means that all instances have to be allocated on the heap, with the new operator to prevent segmentation faults.
 * If allocated on the stack, the variable must never go out of scope.
 * DO NOT ALLOCATE WITH malloc
 */
namespace neda {

    struct Cursor;
    //Note: CursorInfo is the specific position of the cursor on the display, with x and y coordinates
    //CursorLocation can either be START or END and is used to get a cursor at the start or end of the expr.
    struct CursorInfo;
    typedef bool CursorLocation;
    constexpr CursorLocation CURSORLOCATION_START = 0;
    constexpr CursorLocation CURSORLOCATION_END = 1;

    enum class ExprType : uint8_t {
        NULL_TYPE,
        STRING,
        CONTAINER,
        FRACTION,
        EXPONENT,
        L_BRACKET,
        R_BRACKET,
        RADICAL,
        SUPERSCRIPT,
        SUBSCRIPT,
        SIGMA_PI,
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
	class Expr {
	public:
        //The width, height, x and y coordinates are all cached
		virtual void computeWidth() = 0;
		virtual void computeHeight() = 0;
	
		uint16_t getWidth();
		uint16_t getHeight();

        virtual void updatePosition(int16_t, int16_t);
        int16_t getX();
        int16_t getY();
	
		virtual uint16_t getTopSpacing() = 0;
	
        //Draws the expr at the specified coords, updating the cached x and y as it goes
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) = 0;
        //Draws the expr at the cached coords
        void draw(lcd::LCD12864&);
        //Draws all expressions that are connected in some way to this one. e.g. its parents, siblings, grandparents, etc.
        void drawConnected(lcd::LCD12864&);
        //Gets the one top-level expr, a direct parent of this expr that has no parent
        Expr* getTopLevel();
	
		virtual ~Expr() {};

        Expr *parent = nullptr;
        virtual void left(Expr*, Cursor&);
        virtual void right(Expr*, Cursor&);
        virtual void up(Expr*, Cursor&);
        virtual void down(Expr*, Cursor&);
        virtual void getCursor(Cursor&, CursorLocation) = 0;

        virtual ExprType getType() = 0;
	
	protected:
		uint16_t exprWidth;
		uint16_t exprHeight;
        int16_t x;
        int16_t y;
	};
	
	/*
     * The String is a bottom-level expression that is simply a string, and in this case, implemented with a DynamicArray<char>.
     * Being so basic, String does not have any children; its contents are simply a string and nothing else.
     */
	class String : public Expr {
	public:
		//Constructor from string, copy constructor and default constructor
		String(const char *contents) : contents(new DynamicArray<char>(contents, strlen(contents))) {
			computeWidth();
			computeHeight();
		}
		String(const String &other) : contents(other.contents) {
			computeWidth();
			computeHeight();
		}
        String(DynamicArray<char> *contents) : contents(contents) {
            computeWidth();
            computeHeight();
        }
		String() : contents(new DynamicArray<char>()) {
			computeWidth();
			computeHeight();
		}
		
		void addChar(char);
        uint16_t length();

        void addAtCursor(char, Cursor&);
        void removeAtCursor(Cursor&);
        void drawCursor(lcd::LCD12864&, const Cursor&);
        void getCursorInfo(const Cursor&, CursorInfo&);
        String* beforeCursor(const Cursor&);
        String* afterCursor(const Cursor&);
        void merge(const String*);
		
		virtual void computeWidth() override;
		virtual void computeHeight() override;
        virtual uint16_t getTopSpacing() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		virtual ~String() override;

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::STRING;
        }

        virtual void updatePosition(int16_t, int16_t) override;

        static bool isEmptyString(Expr*);
	
	protected:
		DynamicArray<char> *contents;
	};
	
	/*
     * The Container is an expression that serves as a container for a bunch of other expressions.
     * Containers have special logic in their drawing code that make sure everything lines up using the top spacing.
     */
	class Container : public Expr {
	public:
		//Constructor from dynamic array of Expression pointers, copy constructor and default constructor
		Container(const DynamicArray<Expr*> &exprs) : contents(exprs) {
            for(Expr* ex : exprs) {
                ex->parent = this;
            }
			computeWidth();
			computeHeight();
		}
		Container(const Container &other) : contents(other.contents) {
            for(Expr* ex : contents) {
                ex->parent = this;
            }
			computeWidth();
			computeHeight();
		}
		Container() : contents() {
			computeWidth();
			computeHeight();
		}

        static const uint16_t EMPTY_EXPR_WIDTH = 5;
        static const uint16_t EMPTY_EXPR_HEIGHT = 9;
		
		void addExpr(Expr*);
        void removeExpr(uint16_t);
        void replaceExpr(uint16_t, Expr*);
        void addAt(uint16_t, Expr*);
        uint16_t indexOf(Expr*);
        DynamicArray<Expr*>* getContents();
			
        virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		virtual ~Container();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::CONTAINER;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
	protected:
		DynamicArray<Expr*> contents;
        void recomputeBrackets();
	};
	
	//Fraction
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

        virtual ExprType getType() override {
            return ExprType::FRACTION;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
	protected:
		Expr *numerator;
		Expr *denominator;
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
        //Do nothing
        //Realistically this method is never going to be called on LeftBracket anyways
        virtual void getCursor(Cursor &cursor, CursorLocation location) override {}
        virtual ExprType getType() override {
            return ExprType::L_BRACKET;
        }
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
        //Do nothing
        //Realistically this method is never going to be called on RightBracket anyways
        virtual void getCursor(Cursor &cursor, CursorLocation location) override {}
        virtual ExprType getType() override {
            return ExprType::R_BRACKET;
        }
    };

	
	//n-th root expression
	class Radical : public Expr {
	public:
		Radical(Expr *contents, Expr *n) : contents(contents), n(n) {
            contents->parent = this;
            n->parent = this;
			computeWidth();
			computeHeight();
		}
		Radical() : contents(nullptr), n(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		static const uint16_t CONTENTS_N_OVERLAP = 7;
		static const uint16_t SIGN_N_OVERLAP = 1;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		Expr* getContents();
		Expr* getN();
		void setContents(Expr*);
		void setN(Expr*);
		
		virtual ~Radical();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::RADICAL;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
	protected:
		Expr *contents, *n;
	};
	
    //Exponent
    class Exponent : public Expr {
    public:
        Exponent(Expr *base, Expr *exponent) : base(base), exponent(exponent) {
            base->parent = this;
            exponent->parent = this;
            computeWidth();
            computeHeight();
        }
        Exponent() : base(nullptr), exponent(nullptr) {
            computeWidth();
            computeHeight();
        }

        static const uint16_t BASE_EXPONENT_OVERLAP = 4;

        virtual uint16_t getTopSpacing() override;
        virtual void computeWidth() override;
        virtual void computeHeight() override;
        virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;

        Expr* getBase();
        Expr* getExponent();
        void setBase(Expr*);
        void setExponent(Expr*);

        virtual ~Exponent();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void up(Expr*, Cursor&) override;
        virtual void down(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::EXPONENT;
        }

        virtual void updatePosition(int16_t, int16_t) override;

    protected:
        Expr *base;
        Expr *exponent;
    };
    
    //
    class Superscript : public Expr {
    public:
        Superscript(Expr *contents) : contents(contents) {
            computeWidth();
            computeHeight();
        }
        Superscript() : contents(nullptr) {
            computeWidth();
            computeHeight();
        }

        static const uint16_t OVERLAP = 4;

        virtual uint16_t getTopSpacing() override;
        virtual void computeWidth() override;
        virtual void computeHeight() override;
        virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;

        void setContents(Expr*);

        virtual ~Superscript();

        virtual ExprType getType() override {
            return ExprType::SUPERSCRIPT;
        }

        virtual void updatePosition(int16_t, int16_t) override;

    protected:
        Expr *contents;
    };

	//Subscript
	class Subscript : public Expr {
	public:
		Subscript(Expr *contents, Expr *subscript) : contents(contents), subscript(subscript) {
            contents->parent = this;
            subscript->parent = this;
			computeWidth();
			computeHeight();
		}
		Subscript() : contents(nullptr), subscript(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		static const uint16_t CONTENTS_SUBSCRIPT_OVERLAP = 4;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		Expr* getContents();
		Expr* getSubscript();
		void setContents(Expr*);
		void setSubscript(Expr*);
		
		virtual ~Subscript();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::SUBSCRIPT;
        }

        virtual void updatePosition(int16_t, int16_t) override;
		
	protected:
		Expr *contents, *subscript;
	};
	
	//Summation (Sigma) or Product (Pi)
	class SigmaPi : public Expr {
	public:
		SigmaPi(const lcd::LCD12864Image &symbol, Expr *start, Expr *finish, Expr *contents) : symbol(symbol), start(start), finish(finish) {
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

        static const uint16_t CONTENT_SYMBOL_OVERLAP = 12;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		Expr* getStart();
		Expr* getFinish();
		Expr* getContents();
		void setStart(Expr *start);
		void setFinish(Expr *finish);
		void setContents(Expr *contents);
		
		virtual ~SigmaPi();

        virtual void right(Expr*, Cursor&) override;
        virtual void up(Expr*, Cursor&) override;
        virtual void down(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::SIGMA_PI;
        }

        virtual void updatePosition(int16_t, int16_t) override;
		
	protected:
		const lcd::LCD12864Image &symbol;
		Expr *start, *finish, *contents;
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
     * Cursors can only be inside Strings as they make no sense elsewhere.
     */
    struct Cursor {
        String *expr;
        uint16_t index;

        void draw(lcd::LCD12864& dest) {
            expr->drawCursor(dest, *this);
        }
        void addChar(char ch) {
            expr->addAtCursor(ch, *this);
        }
        void removeChar() {
            expr->removeAtCursor(*this);
        }
        void left() {
            expr->left(nullptr, *this);
        }
        void right() {
            expr->right(nullptr, *this);
        }
        void up() {
            expr->up(nullptr, *this);
        }
        void down() {
            expr->down(nullptr, *this);
        }
        void getInfo(CursorInfo &info) {
            expr->getCursorInfo(*this, info);
        }
    };
}

#endif
