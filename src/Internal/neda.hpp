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
        BRACKET,
        RADICAL,
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

        virtual void updatePosition(int16_t, int16_t) = 0;
        int16_t getX();
        int16_t getY();
	
		virtual uint16_t getTopSpacing() = 0;
	
        //Draws the expr at the specified coords, updating the cached x and y as it goes
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) = 0;
        //Draws the expr at the cached coords
        void draw(lcd::LCD12864&);
        //Draws all expressions that are connected in some way to this one. e.g. its parents, siblings, grandparents, etc.
        void drawConnected(lcd::LCD12864&);
	
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
     * The StringExpr is a bottom-level expression that is simply a string, and in this case, implemented with a DynamicArray<char>.
     * Being so basic, StringExpr does not have any children; its contents are simply a string and nothing else.
     */
	class StringExpr : public Expr {
	public:
		//Constructor from string, copy constructor and default constructor
		StringExpr(const char *contents) : contents(new DynamicArray<char>(contents, strlen(contents))) {
			computeWidth();
			computeHeight();
		}
		StringExpr(const StringExpr &other) : contents(other.contents) {
			computeWidth();
			computeHeight();
		}
        StringExpr(DynamicArray<char> *contents) : contents(contents) {
            computeWidth();
            computeHeight();
        }
		StringExpr() : contents(new DynamicArray<char>()) {
			computeWidth();
			computeHeight();
		}
		
		void addChar(char);
        void addAtCursor(char, Cursor&);
        void removeAtCursor(Cursor&);
        void drawCursor(lcd::LCD12864&, const Cursor&);
        void getCursorInfo(const Cursor&, CursorInfo&);
        StringExpr* beforeCursor(const Cursor&);
        StringExpr* afterCursor(const Cursor&);
		
		virtual void computeWidth() override;
		virtual void computeHeight() override;
        virtual uint16_t getTopSpacing() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		virtual ~StringExpr() override;

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::STRING;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
	protected:
		DynamicArray<char> *contents;

        //ContainerExpr needs to know the length of contents for its empty box behavior
        friend class ContainerExpr;
	};
	
	/*
     * The ContainerExpr is an expression that serves as a container for a bunch of other expressions.
     * ContainerExprs have special logic in their drawing code that make sure everything lines up using the top spacing.
     */
	class ContainerExpr : public Expr {
	public:
		//Constructor from dynamic array of Expression pointers, copy constructor and default constructor
		ContainerExpr(const DynamicArray<Expr*> &exprs) : contents(exprs) {
            for(Expr* ex : exprs) {
                ex->parent = this;
            }
			computeWidth();
			computeHeight();
		}
		ContainerExpr(const ContainerExpr &other) : contents(other.contents) {
            for(Expr* ex : contents) {
                ex->parent = this;
            }
			computeWidth();
			computeHeight();
		}
		ContainerExpr() : contents() {
			computeWidth();
			computeHeight();
		}

        static const uint16_t EMPTY_EXPR_WIDTH = 5;
        static const uint16_t EMPTY_EXPR_HEIGHT = 9;
		
		void addExpr(Expr*);
        void removeExpr(Expr*);
        void replaceExpr(Expr*, Expr*);
        void addAfter(Expr*, Expr*);
        void addAfter(Expr*, Expr**, uint16_t);
			
        virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		virtual ~ContainerExpr();

        virtual void left(Expr*, Cursor&) override;
        virtual void right(Expr*, Cursor&) override;
        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::CONTAINER;
        }

        virtual void updatePosition(int16_t, int16_t) override;
	
	protected:
		DynamicArray<Expr*> contents;
	};
	
	//Fraction
	class FractionExpr : public Expr {
	public:
		FractionExpr(Expr *numerator, Expr *denominator) : numerator(numerator), denominator(denominator) {
            numerator->parent = this;
            denominator->parent = this;
			computeWidth();
			computeHeight();
		}
		FractionExpr() : numerator(nullptr), denominator(nullptr) {
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
		
		virtual ~FractionExpr();

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
	
	//Exponent
	class ExponentExpr : public Expr {
	public:
		ExponentExpr(Expr *base, Expr *exponent) : base(base), exponent(exponent) {
            base->parent = this;
            exponent->parent = this;
			computeWidth();
			computeHeight();
		}
		ExponentExpr() : base(nullptr), exponent(nullptr) {
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
		
		virtual ~ExponentExpr();

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
	
	//An expression in brackets
	class BracketExpr : public Expr {
	public:
		BracketExpr(Expr *contents) : contents(contents) {
            contents->parent = this;
			computeWidth();
			computeHeight();
		}
		BracketExpr() : contents(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, int16_t, int16_t) override;
		
		Expr* getContents();
		void setContents(Expr*);
		
		virtual ~BracketExpr();

        virtual void getCursor(Cursor&, CursorLocation) override;

        virtual ExprType getType() override {
            return ExprType::BRACKET;
        }

        virtual void updatePosition(int16_t, int16_t) override;
		
	protected:
		Expr *contents;
	};
	
	//n-th root expression
	class RadicalExpr : public Expr {
	public:
		RadicalExpr(Expr *contents, Expr *n) : contents(contents), n(n) {
            contents->parent = this;
            n->parent = this;
			computeWidth();
			computeHeight();
		}
		RadicalExpr() : contents(nullptr), n(nullptr) {
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
		
		virtual ~RadicalExpr();

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
	
	//Subscript
	class SubscriptExpr : public Expr {
	public:
		SubscriptExpr(Expr *contents, Expr *subscript) : contents(contents), subscript(subscript) {
            contents->parent = this;
            subscript->parent = this;
			computeWidth();
			computeHeight();
		}
		SubscriptExpr() : contents(nullptr), subscript(nullptr) {
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
		
		virtual ~SubscriptExpr();

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
	class SigmaPiExpr : public Expr {
	public:
		SigmaPiExpr(const lcd::LCD12864Image &symbol, Expr *start, Expr *finish, Expr *contents) : symbol(symbol), start(start), finish(finish) {
			start->parent = this;
            finish->parent = this;
            contents->parent = this;
            computeWidth();
			computeHeight();
		}
		SigmaPiExpr(const lcd::LCD12864Image &symbol) : symbol(symbol), start(nullptr), finish(nullptr), contents(nullptr) {
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
		
		virtual ~SigmaPiExpr();

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
     * Cursors can only be inside StringExprs as they make no sense elsewhere.
     */
    struct Cursor {
        StringExpr *expr;
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
