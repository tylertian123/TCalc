#ifndef __SYS_H__
#define __SYS_H__
#include "stm32f10x.h"

#define SYSCLK_Source_HSI 0x00
#define SYSCLK_Source_HSE 0x04
#define SYSCLK_Source_PLL 0x08

namespace sys {
	void initNVIC();
}

#endif
