#include "lcdbase.hpp"
#include "usart.hpp"

namespace lcd {
	
	const GPIOConfig LCDBase::READ_CONFIG = { GPIO_Mode_IPU, GPIO_Speed_2MHz };
	const GPIOConfig LCDBase::WRITE_CONFIG = { GPIO_Mode_Out_PP, GPIO_Speed_2MHz };
	
	#define LCD_WAITBUSY \
		if(!waitForBusyFlag()) \
			return false
	#define LCD_EDELAY delay::cycles(LCD_ENABLE_DELAY)
	#define INIT_I(x) x.init(GPIO_Mode_IPU, GPIO_Speed_2MHz)
	#define INIT_O(x) x.init(GPIO_Mode_Out_PP, GPIO_Speed_2MHz)
	
	void LCDBase::initGPIO() {
		//Initialize the pins for output first
		INIT_O(RS);
		INIT_O(RW);
		INIT_O(E);
		
		setGPIOMode(WRITE_CONFIG);
		
		//Reset the data pins just in case
		setDataPort(0x00);
		RS = false;
		RW = false;
		E = false;
	}
	void LCDBase::setGPIOMode(const GPIOConfig &config) {
		D0.init(config);
		D1.init(config);
		D2.init(config);
		D3.init(config);
		D4.init(config);
		D5.init(config);
		D6.init(config);
		D7.init(config);
	}
	
	uint32_t LCDBase::getTimeout() {
		return this->timeout;
	}
	void LCDBase::setTimeout(uint32_t t) {
		timeout = t;
	}
	
	//These functions set and read from the data port
	void LCDBase::setDataPort(uint8_t data) {
		D0 = data & 0x01;
		D1 = data & 0x02;
		D2 = data & 0x04;
		D3 = data & 0x08;
		D4 = data & 0x10;
		D5 = data & 0x20;
		D6 = data & 0x40;
		D7 = data & 0x80;
	}
	uint8_t LCDBase::readDataPort() {
		setGPIOMode(READ_CONFIG);
		uint8_t result = D0 << 0 | D1 << 1 | D2 << 2 | D3 << 3 | D4 << 4 | D5 << 5 | D6 << 6 | D7 << 7;
		setGPIOMode(WRITE_CONFIG);
		return result;
	}
	
	/*
	 * RS selects whether a command or data will be sent. High - Data, Low - Command
	 * RW selects whether the operation is a read or write operation. High - Read, Low - Write
	 * E enables the LCD by generating a pulse
	 */
	bool LCDBase::waitForBusyFlag() {
		D7 = true;
		RS = false;
		RW = true;
		E = true;
		LCD_EDELAY;
		uint32_t timeoutCounter = 0;
		//Initialize to read the busy flag
		INIT_I(D7);
		//Wait until the pin is cleared
		while(D7) {
			timeoutCounter++;
			delay::us(1);
			//Handle timeout
			if(timeoutCounter > timeout) {
				//Make sure to reset enable pin after
				E = false;
				INIT_O(D7);
				return false;
			}
			E = false;
			LCD_EDELAY;
			E = true;
		}
		E = false;
		INIT_O(D7);
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
	//The busy flag cannnot be checked before initialization, thus delays are used instead of busy flag checking
	void LCDBase::writeCommandNoWait(uint8_t cmd) {
		RS = false;
		RW = false;
		setDataPort(cmd);
		E = true;
		LCD_EDELAY;
		E = false;
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
		for(uint16_t i = 0; str[i] != '\0'; i ++) {
			if(!writeData(str[i])) {
				return false;
			}
		}
		return true;
	}
	
	#undef LCD_WAITBUSY
	#undef LCD_EDELAY
	#undef INIT_I
	#undef INIT_O
}
