#ifndef __LCD1602_H__
#define __LCD1602_H__
#include "stm32f10x.h"
#include "lcdbase.hpp"

namespace lcd {
	
	class LCD1602 : public LCDBase {
	public:
		LCD1602(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin D0, GPIOPin D1, GPIOPin D2, GPIOPin D3, GPIOPin D4, 
			GPIOPin D5, GPIOPin D6, GPIOPin D7, uint32_t timeout = 1000000) :
			LCDBase(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7, timeout) {}
		
		virtual bool init() override;
		virtual bool setCursor(uint8_t, uint8_t) override;
		
		enum Command : uint8_t {
			CLEAR = 0x01,					//0000 0001
			HOME = 0x02,					//0000 0010
			
			ENTRY_CURSOR_SHIFT_RIGHT = 0x06,//0000 0110
			ENTRY_CURSOR_SHIFT_LEFT = 0x04, //0000 0100
			ENTRY_SCREEN_SHIFT_LEFT = 0x07, //0000 0111
			
			DISPLAY_OFF = 0x08,				//0000 1000
			DISPLAY_ON_CURSOR_OFF = 0x0C, 	//0000 1100
			DISPLAY_ON_CURSOR_BLINK = 0x0F, //0000 1111
			DISPLAY_ON_CURSOR_SOLID = 0x0E, //0000 1110
			
			CURSOR_SHIFT_RIGHT = 0x14,		//0001 0100
			CURSOR_SHIFT_LEFT = 0x10,		//0001 0000
			SCREEN_SHIFT_RIGHT = 0x1C,		//0001 1100
			SCREEN_SHIFT_LEFT = 0x18,		//0001 1000
			
			EIGHT_BIT_TWO_LINES = 0x38,		//0011 1000
			EIGHT_BIT_ONE_LINE = 0x30,		//0011 0000
			FOUR_BIT_TWO_LINES = 0x28,		//0010 1000
			FOUR_BIT_ONE_LINE = 0x20,		//0010 0000
		};
		
		bool clear();
		bool home();
	};
}

#endif
