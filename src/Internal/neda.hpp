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
		virtual uint16_t getWidth() = 0;
		virtual uint16_t getHeight() = 0;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) = 0;
	};
	
	//Bottom-level expression that is just a string.
	class StringExpression : public Expression {
	public:
		//Constructor from string, copy constructor and default constructor
		StringExpression(const char *contents) : contents(contents, strlen(contents)) {}
		StringExpression(const StringExpression &other) : contents(other.contents) {}
		StringExpression() : contents() {}
		
		void addChar(char);
			
		virtual uint16_t getWidth() override;
		virtual uint16_t getHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
	
	protected:
		DynamicArray<char> contents;
	};
	
	//An expression that contains multiple expressions.
	class ContainerExpression : public Expression {
	public:
		//Constructor from dynamic array of expression pointers, copy constructor and default constructor
		ContainerExpression(const DynamicArray<Expression*> &expressions) : contents(expressions) {}
		ContainerExpression(const ContainerExpression &other) : contents(other.contents) {}
		ContainerExpression() : contents() {}
		
		void addExpr(Expression*);
			
		virtual uint16_t getWidth() override;
		virtual uint16_t getHeight() override;
		virtual void draw(lcd::LCD12864&, uint16_t, uint16_t) override;
	
	protected:
		DynamicArray<Expression*> contents;
	};
	
	//Fraction
	class FractionExpression : public Expression {
	public:
		FractionExpression(Expression *numerator, Expression *denominator) : numerator(numerator), denominator(denominator) {}
		FractionExpression() : numerator(), denominator() {}
			
		virtual uint16_t getWidth() override;
		virtual uint16_t getHeight() override;
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
