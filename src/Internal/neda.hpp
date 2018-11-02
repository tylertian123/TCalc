#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"

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
		virtual uint16_t getWidth() override;
		virtual uint16_t getHeight() override;
		virtual bool draw(uint16_t, uint16_t) override;
	};
}

#endif