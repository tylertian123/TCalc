#ifndef __NTOA_H__
#define __NTOA_H__

#include "stm32f10x.h"

uint8_t ltoa(int64_t, char*);
uint8_t ftoa(double, char*, uint8_t, char echar = 'e');

#endif
