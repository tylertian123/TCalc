#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f10x.h"
#include "delay.hpp"
#include "gpiopin.hpp"

#define LCD_ENABLE_DELAY 10 // Cycles
#define LCD_PRINTF_BUFFER_SIZE 128 // Characters

namespace lcd {
	
	class LCDBase {
	public:
		LCDBase(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin D0, GPIOPin D1, GPIOPin D2, GPIOPin D3, GPIOPin D4,
				GPIOPin D5, GPIOPin D6, GPIOPin D7, uint32_t timeout = 1000000) :
				RS(RS), RW(RW), E(E), D0(D0), D1(D1), D2(D2), D3(D3), D4(D4), D5(D5), D6(D6), D7(D7), timeout(timeout), FOUR_WIRE_INTERFACE(false) {
			initGPIO();
		}
		LCDBase(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin D4, GPIOPin D5, GPIOPin D6, GPIOPin D7, uint32_t timeout = 1000000) :
				RS(RS), RW(RW), E(E), D4(D4), D5(D5), D6(D6), D7(D7), timeout(timeout), FOUR_WIRE_INTERFACE(true) {
			initGPIO();
		}
	
		virtual bool init() = 0;
		virtual bool setCursor(uint8_t, uint8_t) = 0;
		virtual bool clear() = 0;
		virtual bool home() = 0;
		
		virtual uint32_t getTimeout();
		virtual void setTimeout(uint32_t);
	
		virtual bool writeCommand(uint8_t);
		virtual bool writeData(uint8_t);
		virtual bool readData(uint8_t&);
		virtual bool writeString(const char *);
		virtual bool printf(const char *, ...);
	
	protected:
		GPIOPin RS, RW, E;
		GPIOPin D0, D1, D2, D3, D4, D5, D6, D7;
		uint32_t timeout;
	
		virtual bool waitForBusyFlag();
		
		virtual void writeCommandNoWait(uint8_t);
	
		virtual void setDataPort(uint8_t);
		virtual uint8_t readDataPort();
	
		virtual void setGPIOMode(const GPIOConfig&);
		
		const bool FOUR_WIRE_INTERFACE;
	
		static const GPIOConfig READ_CONFIG;
		static const GPIOConfig WRITE_CONFIG;
		
	
	private:
		void initGPIO();
	};
}

#endif
