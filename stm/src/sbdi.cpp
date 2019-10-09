#include "sbdi.hpp"
#include "delay.hpp"
#include "util.hpp"

#define SBDI_CLK_DELAY_US 50

namespace sbdi {
	
	void Receiver::init() {
		EN.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		CLK.init(GPIO_Mode_Out_PP, GPIO_Speed_10MHz);
        // Clock is always high
        CLK.set(1);
	}

    bool Receiver::receivePending() {
        return !EN;
    }

    void Receiver::receive() {
        // If the enable line is high then skip this
        if(EN) {
            return;
        }

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
