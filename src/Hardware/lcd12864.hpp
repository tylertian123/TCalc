#ifndef __LCD12864_H__
#define __LCD12864_H__
#include "stm32f10x.h"
#include "lcdbase.hpp"
#include <cstring>

namespace lcd {
	
	class LCD12864Image {
	public:
		const uint8_t *data;
		uint16_t bytesWide;
		uint16_t width;
		uint16_t height;
		
		LCD12864Image(const uint8_t *data, uint8_t bytesWide, uint8_t width, uint8_t height) : data(data), bytesWide(bytesWide), width(width), height(height) {}
	};
	
	class LCD12864 : public LCDBase {
	public:
		LCD12864(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin D0, GPIOPin D1, GPIOPin D2, GPIOPin D3, GPIOPin D4, 
			GPIOPin D5, GPIOPin D6, GPIOPin D7, uint32_t timeout = 1000000) :
			LCDBase(RS, RW, E, D0, D1, D2, D3, D4, D5, D6, D7, timeout) {}
		LCD12864(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin D4, GPIOPin D5, GPIOPin D6, GPIOPin D7, uint32_t timeout = 1000000) :
			LCDBase(RS, RW, E, D4, D5, D6, D7, timeout) {}	
		
		virtual bool init() override;
		virtual bool setCursor(uint8_t, uint8_t) override;
		virtual bool clear() override;
		virtual bool home() override;
		
		enum Command {
			CLEAR = 0x01,					//0000 0001
			HOME = 0x02,					//0000 0010
			
			ENTRY_CURSOR_SHIFT_RIGHT = 0x06,//0000 0110
			ENTRY_CURSOR_SHIFT_LEFT = 0x04,	//0000 0100
			ENTRY_SCREEN_SHIFT_LEFT = 0x07,	//0000 0111
			
			DISPLAY_OFF = 0x08,				//0000 1000				
			DISPLAY_ON_CURSOR_OFF = 0x0C,	//0000 1100
			DISPLAY_ON_CURSOR_BLINK = 0x0F,	//0000 1111
			DISPLAY_ON_CURSOR_SOLID = 0x0E,	//0000 1110
			CURSOR_OFF = DISPLAY_ON_CURSOR_OFF,
			CURSOR_BLINK = DISPLAY_ON_CURSOR_BLINK,
			CURSOR_SOLID = DISPLAY_ON_CURSOR_SOLID,
			
			CURSOR_SHIFT_RIGHT = 0x14, 		//0001 0100
			CURSOR_SHIFT_LEFT = 0x10,		//0001 0000
			SCREEN_SHIFT_RIGHT = 0x1C,		//0001 1100
			SCREEN_SHIFT_LEFT = 0x18,		//0001 1000
			
			NORMAL_CMD_8BIT = 0x30,			//0011 0000
			EXT_CMD_8BIT = 0x34,			//0011 0100
			NORMAL_CMD_4BIT = 0x20,			//0010 0000
			EXT_CMD_4BIT = 0x24,			//0010 0100
			
			EXT_STANDBY = 0x01,				//0000 0001
			
			EXT_ENABLE_SCROLL = 0x03,		//0000 0011
			EXT_ENABLE_RAM = 0x02, 			//0000 0010
			
			EXT_REVERSE_0 = 0x04,			//0000 0100
			EXT_REVERSE_1 = 0x05,			//0000 0101
			EXT_REVERSE_2 = 0x06,			//0000 0110
			EXT_REVERSE_3 = 0x07,			//0000 0111
			
			EXT_GRAPHICS_ON_8BIT = 0x36,	//0011 0110
			EXT_GRAPHICS_OFF_8BIT = 0x34,	//0011 0100
			EXT_GRAPHICS_ON_4BIT = 0x26,	//0010 0110
			EXT_GRAPHICS_OFF_4BIT = 0x24,	//0010 0100
			
		};
		
		bool isUsingExtended();
		bool useExtended();
		bool useBasic();
		
		bool isDrawing();
		bool startDraw();
		bool endDraw();
		
		bool clearDrawing();
		bool updateDrawing();
		void clearDrawingBuffer();
		void setPixel(int16_t, int16_t, bool);
		void ORDrawBufferByte(uint16_t, uint16_t, uint8_t);
        void ANDDrawBufferByte(uint16_t, uint16_t, uint8_t);
		void drawImage(int16_t, int16_t, const LCD12864Image&);
		void drawLine(int16_t, int16_t, int16_t, int16_t);
        void drawString(int16_t, int16_t, const char*);
	
	protected:
		bool extendedCmd = false;
		bool drawing = false;
	
		//dispBuf stores what's currently being displayed 
		//drawBuf is what we're working on
		//Total: 2KB of RAM
		//First index is row and second is column
		uint16_t drawBuf[32][16] = { 0 };
		uint16_t dispBuf[32][16] = { 0 };
	};
}

#endif
