#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"
#include <string>

//Nested Expression Display Algorithm
namespace neda {
	
	//Base Expression class
	class Expression {
	public:
		virtual uint16_t getWidth() = 0;
		virtual uint16_t getHeight() = 0;
		virtual bool draw(uint16_t, uint16_t) = 0;
	};
	
	
	class BasicExpression : public Expression {
	public:
		BasicExpression(const char *contents) : contents(contents) {}
		BasicExpression(const std::string contents) : contents(contents) {}
		virtual uint16_t getWidth() override;
		virtual uint16_t getHeight() override;
		virtual bool draw(uint16_t, uint16_t) override;
		
		void addChar(char);
	protected:
		std::string contents;
	};
}

#endif
