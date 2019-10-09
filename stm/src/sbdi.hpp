/*
 * Simple Bi-Directional Interface AKA I2C But Not I2C or I2C2BN
 *
 */

#ifndef __SBDI_H__
#define __SBDI_H__
#include "stm32f10x.h"
#include "gpiopin.hpp"
#include "extipin.hpp"

namespace sbdi {
	class Receiver {
	public:
		typedef void (*Callback)(uint32_t);
		Receiver(GPIOPin EN, GPIOPin DATA, GPIOPin CLK) : EN(EN), DATA(DATA), CLK(CLK) {}
		
		void init();

		uint32_t buffer = 0;

        void receive();
		bool receivePending();
	
	protected:
		GPIOPin EN;
		GPIOPin DATA;
		GPIOPin CLK;
	};
}

#endif
