#ifndef __SBDI_H__
#define __SBDI_H__

#include "8051defs.h"
#include <stdint.h>

/*
 * Simple Bi-Directional Interface AKA I2C But Not I2C or I2C2BN
 * 
 * This bus requires a total of 3 wires:
 * EN - The enable line. This line has a pullup and is pulled down when transmitting data.
 * CLK - The clock line.
 * DAT - The data line.
 * 
 * When idle, EN is high, and both CLK and DAT are high. Transmission is started by the sender puling EN low. 
 * Each time CLK goes from high to low, the level of DAT is read. MSB is sent first. The receiver generates the clock pulses.
 * Transmission ends when EN is restored to high by the sender.
 *
 */
 
void SBDI_BeginTransmission();
void SBDI_EndTransmission();
void SBDI_SendSingleBit(bit);
void SBDI_SendByte(uint8_t);

extern uint32_t SBDI_ReceiveBuffer;
bit SBDI_ReceivePending();
void SBDI_Receive();

#endif
