#include "sbdi.hpp"
#include "delay.hpp"
#include "util.hpp"

#define SBDI_CLK_DELAY_US 50

namespace sbdi {
	
	// Because of how callbacks work, unfortunately Receiver has to be a singleton
	Receiver *receiverInstance;
	
	void Receiver_EN_Callback();
	void Receiver::init() {
		EN.init(GPIO_Speed_10MHz, EXTI_Trigger_Rising_Falling, 0x00, 0x01);
		DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		CLK.init(GPIO_Mode_Out_PP, GPIO_Speed_10MHz);
        // Clock is always high
        CLK.set(1);
		receiverInstance = this;
		
		EN.setCallback(Receiver_EN_Callback);
	}

    void Receiver_EN_Callback() {
        // Falling edge - Transmission started
        if(!receiverInstance->EN) {
            receiverInstance->receivePending = true;
        }
    }

    void Receiver::receive() {
        // Verify and clear receive pending flag
        if(!receivePending) {
            return;
        }
        receivePending = false;

        bool transmissionFailed = false;
        bool parity = 0;
        uint8_t bitCounter = 0;
        __NO_INTERRUPT(
            while(!EN) {
                // Pull clock low
                CLK = 0;
                delay::us(SBDI_CLK_DELAY_US);
                bool data = DATA;
                // Pull clock back high
                CLK = 1;
                delay::us(SBDI_CLK_DELAY_US);

                // Process data
                // If transmission failed, skip this
                if(transmissionFailed) {
                    continue;
                }
                parity ^= data;
                bitCounter++;
                // Every 9th bit is a parity bit
                if(bitCounter == 9) {
                    // Even parity
                    if(parity) {
                        // Transmission failed
                        transmissionFailed = true;
                        // Clear buffer
                        buffer = 0;
                    }

                    parity = 0;
                    bitCounter = 0;
                }
                // Otherwise insert the data bit
                else {
                    buffer <<= 1;
                    buffer |= data;
                }
            }
        )
    }
}
