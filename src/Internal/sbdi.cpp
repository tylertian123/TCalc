#include "sbdi.hpp"

namespace sbdi {
	
	//Because of how callbacks work, unfortunately Receiver has to be a singleton
	Receiver *receiverInstance;
	
	void Receiver_Callback();
	void Receiver::init() {
		EN.init(GPIO_Speed_10MHz, EXTI_Trigger_Falling, 0x03, 0x03);
		DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		CLK.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		receiverInstance = this;
		EN.setCallback(Receiver_Callback);
	}
	
	void Receiver_Callback() {
		receiverInstance->buffer = 0;
		//While the enable line is low
		while(!receiverInstance->EN.getPin()) {
			//Wait until the clock goes from high to low
			while(receiverInstance->CLK) {
				//At any point if the enable line goes back to high then exit
				if(receiverInstance->EN.getPin()) {
					//delay::cycles(360000);
					goto receive_loop_end; //Prepare for a raptor attack
				}
			}
			//delay::cycles(360000);
			//Shift the buffer left and fill the last bit
			receiverInstance->buffer <<= 1;
			receiverInstance->buffer += receiverInstance->DATA;
			
			while(!receiverInstance->CLK);
		}
	receive_loop_end:
		//Call callback if set
		if(receiverInstance->callback) {
			receiverInstance->callback(receiverInstance->buffer);
		}
	}
	
	uint32_t Receiver::getLast() {
		return buffer;
	}
	void Receiver::onReceive(Callback callback) {
		this->callback = callback;
	}
}
