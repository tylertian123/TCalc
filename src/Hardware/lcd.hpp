#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f10x.h"
#include "delay.hpp"
#include "gpiopin.hpp"

#define LCD_ENABLE_DELAY 10 //Cycles

namespace lcd {
	
	class LCDBase {
	public:
		LCDBase(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin BUSY, GPIO_TypeDef *dataPort) :
				RS(RS), RW(RW), E(E), BUSY(BUSY), dataPort(dataPort), timeout(1000000) {}
	
		GPIOPin RS, RW, E, BUSY;
		GPIO_TypeDef *dataPort;
		uint32_t timeout;
	
		void initGPIO();
		//Not yet implemented
		void init();
		
		uint32_t getTimeout();
		void setTimeout(uint32_t);
	
		bool writeCommand(uint8_t cmd);
		bool writeData(uint8_t data);
	
	private:
		bool waitForBusyFlag();
		void setDataPort(uint8_t);
	};
}

#endif
