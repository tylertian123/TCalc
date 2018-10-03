#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f10x.h"
#include "delay.hpp"
#include "gpiopin.hpp"

#define LCD_ENABLE_DELAY 10 //Cycles

namespace lcd {
	
	class LCDBase {
	public:
		LCDBase(GPIOPin RS, GPIOPin RW, GPIOPin E, GPIOPin BUSY, GPIO_TypeDef *dataPort, uint8_t shift = 0, uint32_t timeout = 1000000) :
				RS(RS), RW(RW), E(E), BUSY(BUSY), dataPort(dataPort), shift(shift), timeout(timeout) {
			initGPIO();
		}
	
		virtual bool init() = 0;
		virtual bool setCursor(uint8_t, uint8_t) = 0;
		
		virtual uint32_t getTimeout();
		virtual void setTimeout(uint32_t);
	
		virtual bool writeCommand(uint8_t cmd);
		virtual bool writeData(uint8_t data);
		virtual bool readData(uint8_t &out);
		virtual bool writeString(const char *str);
	
	protected:
		GPIOPin RS, RW, E, BUSY;
		GPIO_TypeDef *dataPort;
		uint8_t shift;
		uint32_t timeout;
	
		virtual bool waitForBusyFlag();
		virtual void setDataPort(uint8_t);
		virtual uint8_t readDataPort();
	private:
		void initGPIO();
	};
}

#endif
