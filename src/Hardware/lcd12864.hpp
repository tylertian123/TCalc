#ifndef __LCD12864_H__
#define __LCD12864_H__
#include "stm32f10x.h"
#include "lcdbase.hpp"

namespace lcd {
	
	class LCD12864 : public LCDBase {
	public:
		LCD12864(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin BUSY, GPIO_TypeDef *dataPort, uint8_t shift = 0, uint32_t timeout = 1000000) :
			LCDBase(RS, RW, E, BUSY, dataPort, shift, timeout) {}
		
		virtual bool init() override;
		virtual bool setCursor(uint8_t, uint8_t) override;
		
		enum Command {
			
		};
	};
}

#endif
