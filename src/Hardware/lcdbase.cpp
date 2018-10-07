#include "lcdbase.hpp"

namespace lcd {
	
	#define LCD_WAITBUSY \
		if(!waitForBusyFlag()) { \
			return false; \
		} 
	#define LCD_EDELAY delay::cycles(LCD_ENABLE_DELAY)
	
	//Initializes one GPIO pin
	void initPin(GPIOPin *pin, GPIOMode_TypeDef mode) {
		RCC_APB2PeriphClockCmd(pin->getRCCPeriph(), ENABLE);
		
		GPIO_InitTypeDef initStruct;
		initStruct.GPIO_Mode = mode;
		initStruct.GPIO_Pin = pin->pin;
		initStruct.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_Init(pin->port, &initStruct);
	}
	
	void LCDBase::initGPIO() {
		initPin(&RS, GPIO_Mode_Out_PP);
		initPin(&RW, GPIO_Mode_Out_PP);
		initPin(&E, GPIO_Mode_Out_PP);
		initPin(&BUSY, GPIO_Mode_IPU);
		
		uint32_t rccPeriph;
		if(dataPort == GPIOA) {
			rccPeriph = RCC_APB2Periph_GPIOA;
		}
		else if(dataPort == GPIOB) {
			rccPeriph = RCC_APB2Periph_GPIOB;
		}
		else if(dataPort == GPIOC) {
			rccPeriph = RCC_APB2Periph_GPIOC;
		}
		else if(dataPort == GPIOD) {
			rccPeriph = RCC_APB2Periph_GPIOD;
		}
		else if(dataPort == GPIOE) {
			rccPeriph = RCC_APB2Periph_GPIOE;
		}
		else if(dataPort == GPIOF) {
			rccPeriph = RCC_APB2Periph_GPIOF;
		}
		else {
			rccPeriph = RCC_APB2Periph_GPIOG;
		}
		
		RCC_APB2PeriphClockCmd(rccPeriph, ENABLE);
		
		GPIO_InitTypeDef initStruct;
		initStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		initStruct.GPIO_Speed = GPIO_Speed_10MHz;
		initStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3 | GPIO_Pin_2 | GPIO_Pin_1 | GPIO_Pin_0;
		GPIO_Init(dataPort, &initStruct);
		
		//Reset the data pins just in case
		setDataPort(0x00);
		RS = false;
		RW = false;
		E = false;
	}
	
	uint32_t LCDBase::getTimeout() {
		return this->timeout;
	}
	void LCDBase::setTimeout(uint32_t t) {
		timeout = t;
	}
	
	//These functions set and read from the data port
	void LCDBase::setDataPort(uint8_t data) {
		//Create a mask
		//The mask will have all bits 1, except for the 8 bits where the data port occupies
		uint16_t mask = ~(0xFF << shift);
		//To not affect the output of other ports, take the current state, mask off some of the bits and OR it with our shifted data
		GPIO_Write(dataPort, (GPIO_ReadInputData(dataPort) & mask) | (data << shift));
	}
	uint8_t LCDBase::readDataPort() {
		//Create a mask
		//Same logic as above, except this mask has all bits 0 except the bits of the data
		uint16_t mask = 0xFF << shift;
		//First mask off unwanted bits and then right-shift back to fit in one byte
		return (GPIO_ReadInputData(dataPort) & mask) >> shift;
	}
	
	/*
	 * RS selects whether a command or data will be sent. High - Data, Low - Command
	 * RW selects whether the operation is a read or write operation. High - Read, Low - Write
	 * E enables the LCD by generating a pulse
	 */
	bool LCDBase::waitForBusyFlag() {
		setDataPort(0xFF);
		RS = false;
		RW = true;
		E = true;
		uint32_t timeoutCounter = 0;
		//Wait until the pin is cleared
		while(BUSY) {
			timeoutCounter++;
			delay::us(1);
			//Handle timeout
			if(timeoutCounter > timeout) {
				//Make sure to reset enable pin after
				E = false;
				return false;
			}
		}
		E = false;
		return true;
	}
	
		
	bool LCDBase::writeCommand(uint8_t cmd) {
		LCD_WAITBUSY;
		
		RS = false;
		RW = false;
		setDataPort(cmd);
		E = true;
		LCD_EDELAY;
		E = false;
		return true;
	}
	bool LCDBase::writeData(uint8_t data) {
		LCD_WAITBUSY;
		
		RS = true;
		RW = false;
		setDataPort(data);
		E = true;
		LCD_EDELAY;
		E = false;
		return true;
	}
	bool LCDBase::readData(uint8_t &out) {
		LCD_WAITBUSY;
		
		RS = true;
		RW = true;
		E = true;
		LCD_EDELAY;
		out = readDataPort();
		E = false;
		return true;
	}
	bool LCDBase::writeString(const char *str) {
		while(*str != '\0') {
			if(!writeData(*str++)) {
				return false;
			}
		}
		return true;
	}
	
	#undef LCD_WAITBUSY
	#undef LCD_EDELAY
}
