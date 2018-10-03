#include "delay.hpp"

#define DELAY_TIMER TIM2
#define DELAY_TIMER_PERIPH RCC_APB1Periph_ ## TIM2
//Note that even though APB1 is set to SYSCLK/2, according to the clock tree the clock source will be multiplied by 2 again if the APB prescaler is not 1.
#define APB1_FREQUENCY 72 //MHz

namespace delay {
	
	void init() {
		RCC_APB1PeriphClockCmd(DELAY_TIMER_PERIPH, ENABLE);
		//Set direction to counting down
		//Enable One-Pulse mode
		DELAY_TIMER->CR1 |= TIM_CR1_DIR | TIM_CR1_OPM;
		//Set prescaler to 3 to divide by 4
		DELAY_TIMER->PSC = 3;
	}
	
	void cycles(uint16_t c) {
		while(c--);
	}
	
	void us(uint16_t microSeconds) {
		//Clear the status register
		DELAY_TIMER->SR = 0x0000;
		DELAY_TIMER->ARR = microSeconds * (APB1_FREQUENCY / 4);
		//Start timer
		DELAY_TIMER->CR1 |= TIM_CR1_CEN;
		//Wait until the update flag is set
		while(!(DELAY_TIMER->SR & TIM_SR_UIF));
		//No need to reset CEN because OPM is enabled
	}
	void ms(uint16_t ms) {
		while(ms --) us((uint16_t) 1000);
	}
	void sec(uint16_t s) {
		while(s --) ms(1000);
	}
}
