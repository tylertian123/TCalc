#include "delay.hpp"
#include "util.hpp"

#define SYSCLK_FREQUENCY 72 // MHz

namespace delay {
	
	void cycles(uint32_t c) {
		while(c--);
	}
	
	void us(uint16_t microSeconds) {
		__NO_INTERRUPT(
			// Set SysTick reload value
			SysTick->LOAD = microSeconds * SYSCLK_FREQUENCY;
			// Clear current value
			SysTick->VAL = 0x00000000;
			// Set SysTick to use system clock, no interrupts and start timer
			SysTick->CTRL = 0x00000005; // 0000 0000 0000 0000 0000 0000 0000 0101
			// Constantly check for timer overflow
			while(!(SysTick->CTRL & 0x00010000)); // 0000 0000 0000 0001 0000 0000 0000 0000
			// Stop SysTick
			SysTick->CTRL = 0x00000004; // 0000 0000 0000 0000 0000 0000 0000 0100
		);
	}
	void ms(uint16_t ms) {
		while(ms --) us((uint16_t) 1000);
	}
	void sec(uint16_t s) {
		while(s --) ms(1000);
	}
}
