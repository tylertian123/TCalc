#ifndef __NEDA_H__
#define __NEDA_H__
#include "stm32f10x.h"
#include "lcd12864.hpp"

namespace neda {
	
	class Expression {
	public:
		virtual uint16_t getWidth() = 0;
		virtual uint16_t getHeight() = 0;
		virtual bool draw(uint16_t x, uint16_t y) = 0;
	};
}

#endif
