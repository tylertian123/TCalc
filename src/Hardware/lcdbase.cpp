#include "lcdbase.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "util.hpp"

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
		// Initialize the pins for output first
		INIT_O(RS);
		INIT_O(RW);
		INIT_O(E);
		
		setGPIOMode(WRITE_CONFIG);
		
		// Reset the data pins just in case
		setDataPort(0x00);
		RS = false;
		RW = false;
		E = false;
	}
	void LCDBase::setGPIOMode(const GPIOConfig &config) {
		if(!FOUR_WIRE_INTERFACE) {
			D0.init(config);
			D1.init(config);
			D2.init(config);
			D3.init(config);
		}
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
	
	// These functions set and read from the data port
	// If in four wire interface, only the lowest 4 bits will be written
	void LCDBase::setDataPort(uint8_t data) {
		if(!FOUR_WIRE_INTERFACE) {
			D0 = data & 0x01;
			D1 = data & 0x02;
			D2 = data & 0x04;
			D3 = data & 0x08;
			D4 = data & 0x10;
			D5 = data & 0x20;
			D6 = data & 0x40;
			D7 = data & 0x80;
		}
		else {
			data &= 0x0F;
			D4 = data & 0x01;
			D5 = data & 0x02;
			D6 = data & 0x04;
			D7 = data & 0x08;
		}
	}
	// If in four wire interface, only a nibble will be read
	uint8_t LCDBase::readDataPort() {
		setGPIOMode(READ_CONFIG);
		uint8_t result = FOUR_WIRE_INTERFACE ? (D7 << 3 | D6 << 2 | D5 << 1 | D4 << 0) : 
				(D0 << 0 | D1 << 1 | D2 << 2 | D3 << 3 | D4 << 4 | D5 << 5 | D6 << 6 | D7 << 7);
		setGPIOMode(WRITE_CONFIG);
		return result;
	}
	
	/*
	 * RS selects whether a command or data will be sent. High - Data, Low - Command
	 * RW selects whether the operation is a read or write operation. High - Read, Low - Write
	 * E enables the LCD by generating a pulse
	 */
	bool LCDBase::waitForBusyFlag() {
        __NO_INTERRUPT(
            D7 = true;
            RS = false;
            RW = true;
            E = true;
            LCD_EDELAY;
            uint32_t timeoutCounter = 0;
            // Initialize to read the busy flag
            INIT_I(D7);
            // Wait until the pin is cleared
            while(D7) {
                timeoutCounter++;
                delay::us(1);
                // Handle timeout
                if(timeoutCounter > timeout) {
                    // Make sure to reset enable pin after
                    E = false;
                    INIT_O(D7);
                    return false;
                }
                
                E = false;
                LCD_EDELAY;
                E = true;
                
                if(FOUR_WIRE_INTERFACE) {
                    LCD_EDELAY;
                    E = false;
                    LCD_EDELAY;
                    E = true;
                }
                    
            }
            E = false;
            INIT_O(D7);
        );
		return true;
	}
	
		
	bool LCDBase::writeCommand(uint8_t cmd) {
		__NO_INTERRUPT(

            LCD_WAITBUSY;
            RS = false;
            RW = false;
            
            if(FOUR_WIRE_INTERFACE) {
                setDataPort(cmd >> 4);
                E = true;
                LCD_EDELAY;
                E = false;
                setDataPort(cmd & 0x0F);
                LCD_EDELAY;
                E = true;
                LCD_EDELAY;
                E = false;
            }
            else {
                setDataPort(cmd);
                E = true;
                LCD_EDELAY;
                E = false;
            }
            
		);

		return true;
	}
	// The busy flag cannot be checked before initialization, thus delays are used instead of busy flag checking
	void LCDBase::writeCommandNoWait(uint8_t cmd) {
		__NO_INTERRUPT(
		
            RS = false;
            RW = false;
            
            if(FOUR_WIRE_INTERFACE) {
                setDataPort(cmd >> 4);
                E = true;
                LCD_EDELAY;
                E = false;
                setDataPort(cmd & 0x0F);
                LCD_EDELAY;
                E = true;
                LCD_EDELAY;
                E = false;
            }
            else {
                setDataPort(cmd);
                E = true;
                LCD_EDELAY;
                E = false;
            }
		
		);
	}
	bool LCDBase::writeData(uint8_t data) {
		__NO_INTERRUPT(
		
            LCD_WAITBUSY;
            RS = true;
            RW = false;
            
            if(FOUR_WIRE_INTERFACE) {
                setDataPort(data >> 4);
                E = true;
                LCD_EDELAY;
                E = false;
                setDataPort(data & 0x0F);
                LCD_EDELAY;
                E = true;
                LCD_EDELAY;
                E = false;
            }
            else {
                setDataPort(data);
                E = true;
                LCD_EDELAY;
                E = false;
            }
            
        );
		return true;
	}
	bool LCDBase::readData(uint8_t &out) {
		__NO_INTERRUPT(
		
            LCD_WAITBUSY;
            RS = true;
            RW = true;
            
            if(FOUR_WIRE_INTERFACE) {
                E = true;
                LCD_EDELAY;
                out = readDataPort() << 4;
                E = false;
                LCD_EDELAY;
                E = true;
                LCD_EDELAY;
                out |= readDataPort() & 0x0F;
                E = false;
            }
            else {
                E = true;
                LCD_EDELAY;
                out = readDataPort();
                E = false;
            }
		
        );
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
	bool LCDBase::printf(const char *fmt, ...) {
		// Buffer used to store formatted string
		char buf[LCD_PRINTF_BUFFER_SIZE] = { 0 };
		std::va_list args;
		va_start(args, fmt);
		// Use vsnprintf to safely format the string and put into the buffer
		vsnprintf(buf, LCD_PRINTF_BUFFER_SIZE, fmt, args);
		return writeString(buf);
	}
	
	#undef LCD_WAITBUSY
	#undef LCD_EDELAY
	#undef INIT_I
	#undef INIT_O
}
