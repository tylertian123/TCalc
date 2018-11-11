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
		virtual void computeWidth() = 0;
		virtual void computeHeight() = 0;
	
		virtual uint16_t getWidth();
		virtual uint16_t getHeight();
	
		virtual uint16_t getTopSpacing() = 0;
	
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) = 0;
	
		virtual ~Expr() {};
	
	protected:
		uint16_t exprWidth;
		uint16_t exprHeight;
	};
	
	/*
     * The StringExpr is a bottom-level expression that is simply a string, and in this case, implemented with a DynamicArray<char>.
     * Being so basic, StringExpr does not have any children; its contents are simply a string and nothing else.
     */
	class StringExpr : public Expr {
	public:
		//Constructor from string, copy constructor and default constructor
		StringExpr(const char *contents) : contents(contents, strlen(contents)) {
			computeWidth();
			computeHeight();
		}
		StringExpr(const StringExpr &other) : contents(other.contents) {
			computeWidth();
			computeHeight();
		}
		StringExpr() : contents() {
			computeWidth();
			computeHeight();
		}
		
		void addChar(char);
		
		virtual void computeWidth() override;
		virtual void computeHeight() override;
        virtual uint16_t getTopSpacing() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		//StringExprs don't need special handling because it doesn't have any children
		virtual ~StringExpr() {}
	
	protected:
		DynamicArray<char> contents;
	};
	
	/*
     * The ContainerExpr is an expression that serves as a container for a bunch of other expressions.
     * ContainerExprs have special logic in their drawing code that make sure everything lines up using the top spacing.
     */
	class ContainerExpr : public Expr {
	public:
		//Constructor from dynamic array of Expression pointers, copy constructor and default constructor
		ContainerExpr(const DynamicArray<Expr*> &Exprs) : contents(Exprs) {
			computeWidth();
			computeHeight();
		}
		ContainerExpr(const ContainerExpr &other) : contents(other.contents) {
			computeWidth();
			computeHeight();
		}
		ContainerExpr() : contents() {
			computeWidth();
			computeHeight();
		}

        static const int EMPTY_CONTAINER_WIDTH = 5;
        static const int EMPTY_CONTAINER_HEIGHT = 9;
		
		void addExpr(Expr*);
			
        virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		virtual ~ContainerExpr();
	
	protected:
		DynamicArray<Expr*> contents;
	};
	
	//Fraction
	class FractionExpr : public Expr {
	public:
		FractionExpr(Expr *numerator, Expr *denominator) : numerator(numerator), denominator(denominator) {
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
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
			
		Expr* getNumerator();
		Expr* getDenominator();
		void setNumerator(Expr*);
		void setDenominator(Expr*);
		
		virtual ~FractionExpr();
	
	protected:
		Expr *numerator;
		Expr *denominator;
	};
	
	//Exponent
	class ExponentExpr : public Expr {
	public:
		ExponentExpr(Expr *base, Expr *exponent) : base(base), exponent(exponent) {
			computeWidth();
			computeHeight();
		}
		ExponentExpr() : base(nullptr), exponent(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		static const int BASE_EXPONENT_OVERLAP = 4;
		
        virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		Expr* getBase();
		Expr* getExponent();
		void setBase(Expr*);
		void setExponent(Expr*);
		
		virtual ~ExponentExpr();
	protected:
		Expr *base;
		Expr *exponent;
	};
	
	//An expression in brackets
	class BracketExpr : public Expr {
	public:
		BracketExpr(Expr *contents) : contents(contents) {
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
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		Expr* getContents();
		void setContents(Expr*);
		
	protected:
		Expr *contents;
	};
	
	//An expression in a square root symbol
	class SqrtExpr : public Expr {
	public:
		SqrtExpr(Expr *contents) : contents(contents) {
			computeWidth();
			computeHeight();
		}
		SqrtExpr() : contents(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		Expr *getContents();
		void setContents(Expr*);
		
	protected:
		Expr *contents;
	};
	
	//n-th root expression
	class RadicalExpr : public Expr {
	public:
		RadicalExpr(Expr *contents, Expr *n) : contents(contents), n(n) {
			computeWidth();
			computeHeight();
		}
		RadicalExpr() : contents(nullptr), n(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		static const int CONTENTS_N_OVERLAP = 7;
		static const int SIGN_N_OVERLAP = 1;
		
		virtual uint16_t getTopSpacing() override;
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		Expr* getContents();
		Expr* getN();
		void setContents(Expr*);
		void setN(Expr*);
	
	protected:
		Expr *contents, *n;
	};
}

#endif
