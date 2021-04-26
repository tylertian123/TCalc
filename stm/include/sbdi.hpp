/*
 * Simple Bi-Directional Interface AKA I2C But Not I2C or I2C2BN
 *
 */

#ifndef __SBDI_H__
#define __SBDI_H__
#include "gpiopin.hpp"
#include "stm32f10x.h"

namespace sbdi {
    class SBDI {
    public:
        SBDI(GPIOPin EN, GPIOPin DATA, GPIOPin CLK) : EN(EN), DATA(DATA), CLK(CLK) {
        }

        void init();

        uint32_t buffer = 0;

        void receive();
        bool receivePending();

        void send();

        void beginSend();
        void sendByte(uint8_t data);
        void endSend();

        void send32(uint32_t data);

    protected:
        GPIOPin EN;
        GPIOPin DATA;
        GPIOPin CLK;
    };
} // namespace sbdi

#endif
