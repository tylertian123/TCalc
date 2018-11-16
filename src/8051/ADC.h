#ifndef __ADC_H__
#define __ADC_H__

void ADC_StartConv(unsigned char);
bit ADC_ConvFin();
unsigned short ADC_GetResult();
unsigned short ADC_SyncConv(unsigned char);

#endif
