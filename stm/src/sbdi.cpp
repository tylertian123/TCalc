#include "sbdi.hpp"
#include "delay.hpp"
#include "util.hpp"

#define SBDI_CLK_DELAY_US 50

namespace sbdi {
	
	void SBDI::init() {
		EN.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		CLK.init(GPIO_Mode_Out_PP, GPIO_Speed_10MHz);
        // Clock is always high
        CLK.set(1);
	}

    bool SBDI::receivePending() {
        return !EN;
    }

    void SBDI::receive() {
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

    void SBDI::beginSend() {
        EN.init(GPIO_Mode_Out_PP, GPIO_Speed_10MHz);
        DATA.init(GPIO_Mode_Out_PP, GPIO_Speed_10MHz);
        CLK.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);

        EN = 0;
        DATA = 1;
    }

    void SBDI::sendByte(uint8_t data) {
        uint8_t mask = 0x80;
        // Use even parity
        bool parity = 0;
        do {
            // Clock is active low
            // Wait until clock is low
            while(CLK);
            DATA = data & mask;
            // Wait until clock is high again
            while(!CLK);
            parity ^= static_cast<bool>(data & mask);
        } while(mask >>= 1);
        // Send parity bit
        while(CLK);
        DATA = parity;
        while(!CLK);
    }

    void SBDI::endSend() {
        EN = 1;
        DATA = 1;

        EN.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		DATA.init(GPIO_Mode_IN_FLOATING, GPIO_Speed_10MHz);
		CLK.init(GPIO_Mode_Out_PP, GPIO_Speed_10MHz);
        // Clock is always high
        CLK.set(1);
    }

    void SBDI::send32(uint32_t data) {
        beginSend();
        sendByte((data >> 24) & 0xFF);
        sendByte((data >> 16) & 0xFF);
        sendByte((data >> 8) & 0xFF);
        sendByte(data & 0xFF);
        endSend();
    }
}
