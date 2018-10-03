#include "lcd.hpp"

namespace lcd {
	
	#define LCD_WAITBUSY \
		if(!waitForBusyFlag()) { \
			return false; \
		} \
	#define LCD_EDELAY delay::cycles(LCD_ENABLE_DELAY)
	
	void initPin(GPIOPin *pin, GPIOMode_TypeDef mode) {
		uint32_t rccPeriph;
		if(pin->port == GPIOA) {
			rccPeriph = RCC_APB2Periph_GPIOA;
		}
		else if(pin->port == GPIOB) {
			rccPeriph = RCC_APB2Periph_GPIOB;
		}
		else if(pin->port == GPIOC) {
			rccPeriph = RCC_APB2Periph_GPIOC;
		}
		else if(pin->port == GPIOD) {
			rccPeriph = RCC_APB2Periph_GPIOD;
		}
		else if(pin->port == GPIOE) {
			rccPeriph = RCC_APB2Periph_GPIOE;
		}
		else if(pin->port == GPIOF) {
			rccPeriph = RCC_APB2Periph_GPIOF;
		}
		else {
			rccPeriph = RCC_APB2Periph_GPIOG;
		}
		
		RCC_APB2PeriphClockCmd(rccPeriph, ENABLE);
		
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
		
		setDataPort(0x00);
		RS = false;
		RW = false;
		E = false;
	}
	void LCDBase::init() {
		#pragma message("LCDBase::init() not implemented")
	}
	
	uint32_t LCDBase::getTimeout() {
		return this->timeout;
	}
	void LCDBase::setTimeout(uint32_t t) {
		timeout = t;
	}
	
	void LCDBase::setDataPort(uint8_t data) {
		GPIO_Write(dataPort, (GPIO_ReadInputData(dataPort) & 0xFF00) | data);
	}
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
		E = false;
	}
	bool LCDBase::writeData(uint8_t cmd) {
		
	}
	
	#undef LCD_WAITBUSY
	#undef LCD_EDELAY
}
