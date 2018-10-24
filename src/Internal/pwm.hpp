#ifndef __PWM_H__
#define __PWM_H__
#include "stm32f10x.h"
#include "gpiopin.hpp"

namespace pwm {
	
	template <uint8_t CHANNEL, uint32_t CLOCK_FREQ, uint32_t PWM_FREQ, uint16_t PWM_STEPS = 0x100>
	class PWMOutput {
	public:
		PWMOutput(TIM_TypeDef *TIMER, uint32_t TIMER_PERIPH, GPIOPin& pin) : TIMER(TIMER) {
			constexpr uint32_t counterFreq = PWM_FREQ * PWM_STEPS;
			constexpr uint16_t pscVal = CLOCK_FREQ / counterFreq - 1;
			constexpr uint16_t arrVal = PWM_STEPS - 1;
			
			RCC_APB1PeriphClockCmd(TIMER_PERIPH, ENABLE);
			TIM_TimeBaseInitTypeDef initStruct;
			TIM_TimeBaseStructInit(&initStruct);
			initStruct.TIM_Period = arrVal;
			initStruct.TIM_Prescaler = pscVal;
			TIM_TimeBaseInit(TIMER, &initStruct);
			
			TIM_OCInitTypeDef ocInitStruct;
			TIM_OCStructInit(&ocInitStruct);
			ocInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
			ocInitStruct.TIM_OutputState = TIM_OutputState_Enable;
			ocInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
			ocInitStruct.TIM_Pulse = 0;
			
			TIM_OCInitFuncs[CHANNEL - 1](TIMER, &ocInitStruct);
			
			pin.init(GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
			pin = false;
		}
		
		void startTimer() {
			TIM_Cmd(TIMER, ENABLE);
		}
		void stopTimer() {
			TIM_Cmd(TIMER, DISABLE);
		}
		
		void set(uint16_t val) {
			TIM_SetCompareFuncs[CHANNEL - 1](TIMER, val);
		}
	
	protected:
		TIM_TypeDef *TIMER;
		static void (*const TIM_OCInitFuncs[4])(TIM_TypeDef *, TIM_OCInitTypeDef *);
		static void (*const TIM_SetCompareFuncs[4])(TIM_TypeDef *, uint16_t);
	};
	
	template <uint8_t CHANNEL, uint32_t CLOCK_FREQ, uint32_t PWM_FREQ, uint16_t PWM_STEPS>
	void (*const PWMOutput<CHANNEL, CLOCK_FREQ, PWM_FREQ, PWM_STEPS>::TIM_OCInitFuncs[4])(TIM_TypeDef *, TIM_OCInitTypeDef *) = {
		TIM_OC1Init, TIM_OC2Init, TIM_OC3Init, TIM_OC4Init
	};
	template <uint8_t CHANNEL, uint32_t CLOCK_FREQ, uint32_t PWM_FREQ, uint16_t PWM_STEPS>
	void (*const PWMOutput<CHANNEL, CLOCK_FREQ, PWM_FREQ, PWM_STEPS>::TIM_SetCompareFuncs[4])(TIM_TypeDef *, uint16_t) = {
		TIM_SetCompare1, TIM_SetCompare2, TIM_SetCompare3, TIM_SetCompare4
	};
}

#endif
