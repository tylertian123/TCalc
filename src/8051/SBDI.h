#ifndef __SBDI_H__
#define __SBDI_H__

/*
 * Simple Bi-Directional Interface
 * 
 * This bus requires a total of 3 wires:
 * EN - The enable line. This line has a pullup and is pulled down when transmitting data.
 * CLK - The clock line.
 * DAT - The data line.
 * 
 * When idle, EN is high, and both CLK and DAT are high. Transmission is started by the sender puling EN low. 
 * Each time CLK goes from high to low, the level of DAT is read. MSB is sent first. The sender generates the clock pulses.
 * Transmission ends when EN is restored to high by the sender.
 *
 * This file only contains functions used for sending data.
 */
 
void SBDI_BeginTransmission();
void SBDI_EndTransmission();
void SBDI_SendSingleBit(bit);
void SBDI_SendByte(unsigned char);

#endif