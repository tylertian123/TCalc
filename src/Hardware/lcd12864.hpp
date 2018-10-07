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
			CLEAR = 0x01,		//0000 0001
			HOME = 0x02,		//0000 0010
			
			ENTRY_CURSOR_SHIFT_RIGHT = 0x0C, //
			ENTRY_CURSOR_SHIFT_LEFT = 0x04,
			ENTRY_SCREEN_SHIFT_LEFT = 0x07,
			
			DISPLAY_OFF 0x08
			DISPLAY_ON_CURSOR_OFF 0x0C
			DISPLAY_ON_CURSOR_BLINK 0x0F
			DISPLAY_ON_CURSOR_ON_NO_BLINK 0x0E

		};
		
		bool clear();
		bool home();
	};
}

#endif
