#include "sbdi.hpp"
#include "delay.hpp"

namespace sbdi {
	
	// Because of how callbacks work, unfortunately Receiver has to be a singleton
	Receiver *receiverInstance;
	bool transmissionStarted = false;
	
	void Receiver_CLK_Callback();
	void Receiver_EN_Callback();
	void Receiver::init() {
		EN.init(GPIO_Speed_10MHz, EXTI_Trigger_Rising_Falling, 0x00, 0x01);
		DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		CLK.init(GPIO_Speed_10MHz, EXTI_Trigger_Falling, 0x00, 0x02);
		receiverInstance = this;
		
		CLK.setCallback(Receiver_CLK_Callback);
		EN.setCallback(Receiver_EN_Callback);
	}
	void Receiver_EN_Callback() {
		// Rising Edge - Transmission over
		if(receiverInstance->EN) {
			transmissionStarted = false;
			// Call callback if set
			if(receiverInstance->callback) {
				receiverInstance->callback(receiverInstance->buffer);
			}
		}
		// Falling Edge - Transmission started
		else {
			transmissionStarted = true;
			receiverInstance->buffer = 0;
		}
	}
	void Receiver_CLK_Callback() {
		receiverInstance->buffer <<= 1;
		receiverInstance->buffer += receiverInstance->DATA;
	}
	
	uint32_t Receiver::getLast() {
		return buffer;
	}
	void Receiver::onReceive(Callback callback) {
		this->callback = callback;
	}
}
