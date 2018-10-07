#ifndef __GPIOPIN_H__
#define __GPIOPIN_H__
#include "stm32f10x.h"

/*
 * A class that represents a GPIO pin
 */
class GPIOPin {
public:
	GPIO_TypeDef *port;
	uint16_t pin;

	GPIOPin(GPIO_TypeDef *port, uint16_t pin) : port(port), pin(pin) {}
	GPIOPin() : port(0), pin(0) {}
	
	virtual void set(const bool &val);
	virtual bool get(void) const;

	GPIOPin& operator=(const bool&);
	
	operator bool() const;
		
	uint32_t getRCCPeriph() const;
};

#endif
