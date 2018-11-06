#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"
#include "dynamarr.hpp"
#include <string.h>

//Nested Expression Display Algorithm
//WARNING: All instances have to be allocated on the heap with new
namespace neda {
	
	//Base Expression class.
	class Expr {
	public:
		virtual void computeWidth() = 0;
		virtual void computeHeight() = 0;
		virtual uint16_t getWidth();
		virtual uint16_t getHeight();
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) = 0;
	
		virtual ~Expr() {};
	
	protected:
		uint16_t exprWidth;
		uint16_t exprHeight;
	};
	
	//Bottom-level Expression that is just a string.
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
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
		
		//StringExprs don't need special handling because it doesn't have any children
		virtual ~StringExpr() {}
	
	protected:
		DynamicArray<char> contents;
	};
	
	//An Expression that contains multiple Exprs.
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
		
		void addExpr(Expr*);
			
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
}

#endif
