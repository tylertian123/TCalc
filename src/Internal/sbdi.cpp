#include "sbdi.hpp"
#include "delay.hpp"

namespace sbdi {
	
	// Because of how callbacks work, unfortunately Receiver has to be a singleton
	Receiver *receiverInstance;
	bool transmissionStarted = false;
    bool transmissionFailed = false;

    uint8_t bitCounter = 0;
    bool parity = 0;
	
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
            // Clear buffer on failure
            if(transmissionFailed) {
                receiverInstance->buffer = 0;
            }
            bitCounter = 0;
            parity = 0;
			// Call callback if exists and transmission didn't fail
			if(!transmissionFailed && receiverInstance->callback) {
				receiverInstance->callback(receiverInstance->buffer);
			}
		}
		// Falling Edge - Transmission started
		else {
			transmissionStarted = true;

            transmissionFailed = false;
            bitCounter = 0;
            parity = 0;

			receiverInstance->buffer = 0;
		}
	}
	void Receiver_CLK_Callback() {
        bool data = receiverInstance->DATA;

        parity ^= data;
        ++bitCounter;

        // Every 9th bit is a parity bit
        if(bitCounter == 9) {
            // Even parity
            if(parity) {
                transmissionFailed = true;
            }

            parity = 0;
            bitCounter = 0;
        }
        else {
            receiverInstance->buffer <<= 1;
            receiverInstance->buffer |= data;
        }
	}
	
	uint32_t Receiver::getLast() {
		return buffer;
	}
	void Receiver::onReceive(Callback callback) {
		this->callback = callback;
	}
}
