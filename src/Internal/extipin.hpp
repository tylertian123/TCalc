#ifndef __EXTI_H__
#define __EXTI_H__
#include "stm32f10x.h"
#include "gpiopin.hpp"

class EXTIPin {

public:
	typedef void (*CallbackFunction)();

	EXTIPin(GPIOPin pin) : pin(pin) {}
	EXTIPin() {}
	
	void init(GPIOSpeed_TypeDef, EXTITrigger_TypeDef, uint8_t, uint8_t);
	void setCallback(CallbackFunction);
		
	GPIOPin getPin();
	
	uint32_t getEXTILine();
	uint8_t getEXTIIRQChannel();

	uint8_t getEXTIPinSource();
	uint8_t getEXTIPortSource();
	
	operator bool() const;
	
	static CallbackFunction EXTI0Callback, EXTI1Callback, EXTI2Callback, EXTI3Callback, 
		EXTI4Callback, EXTI5Callback, EXTI6Callback, EXTI7Callback,
		EXTI8Callback, EXTI9Callback, EXTI10Callback, EXTI11Callback,
		EXTI12Callback, EXTI13Callback, EXTI14Callback, EXTI15Callback;

protected:
	GPIOPin pin;
};

#endif
