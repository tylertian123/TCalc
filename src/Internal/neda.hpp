#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"
#include "dynamarr.hpp"
#include <string.h>

//Nested Expression Display Algorithm
namespace neda {
	
	//Base Expression class.
	class Expression {
	public:
		virtual void computeWidth() = 0;
		virtual void computeHeight() = 0;
		virtual uint16_t getWidth();
		virtual uint16_t getHeight();
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) = 0;
	protected:
		uint16_t exprWidth;
		uint16_t exprHeight;
	};
	
	//Bottom-level expression that is just a string.
	class StringExpression : public Expression {
	public:
		//Constructor from string, copy constructor and default constructor
		StringExpression(const char *contents) : contents(contents, strlen(contents)) {
			computeWidth();
			computeHeight();
		}
		StringExpression(const StringExpression &other) : contents(other.contents) {
			computeWidth();
			computeHeight();
		}
		StringExpression() : contents() {
			computeWidth();
			computeHeight();
		}
		
		void addChar(char);
			
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
	
	protected:
		DynamicArray<char> contents;
	};
	
	//An expression that contains multiple expressions.
	class ContainerExpression : public Expression {
	public:
		//Constructor from dynamic array of expression pointers, copy constructor and default constructor
		ContainerExpression(const DynamicArray<Expression*> &expressions) : contents(expressions) {
			computeWidth();
			computeHeight();
		}
		ContainerExpression(const ContainerExpression &other) : contents(other.contents) {
			computeWidth();
			computeHeight();
		}
		ContainerExpression() : contents() {
			computeWidth();
			computeHeight();
		}
		
		void addExpr(Expression*);
			
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
	
	protected:
		DynamicArray<Expression*> contents;
	};
	
	//Fraction
	class FractionExpression : public Expression {
	public:
		FractionExpression(Expression *numerator, Expression *denominator) : numerator(numerator), denominator(denominator) {
			computeWidth();
			computeHeight();
		}
		FractionExpression() : numerator(nullptr), denominator(nullptr) {
			computeWidth();
			computeHeight();
		}
		
		virtual void computeWidth() override;
		virtual void computeHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
			
		Expression* getNumerator();
		Expression* getDenominator();
		void setNumerator(Expression*);
		void setDenominator(Expression*);
	
	protected:
		Expression *numerator;
		Expression *denominator;
	};
}

#endif
