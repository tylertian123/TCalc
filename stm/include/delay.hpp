#ifndef __DELAY_H__
#define __DELAY_H__
#include "stm32f10x.h"

namespace delay {
	void cycles(uint32_t);
	void sec(uint16_t s);
	void ms(uint16_t ms);
	void us(uint16_t us);
}

#endif
