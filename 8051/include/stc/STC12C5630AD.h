#ifndef __STC12C5630AD_H_
#define __STC12C5630AD_H_

#include "8051defs.h"

/////////////////////////////////////////////////

/* The following is STC additional SFR or change */

/* SFR(AUXR, 0x8e); */
/* SFR(IPH, 0xb7); */

/* Watchdog Timer Register */
SFR(WDT_CONTR, 0xe1);    

/* ISP_IAP_EEPROM Register */
SFR(ISP_DATA, 0xe2);
SFR(ISP_ADDRH, 0xe3);
SFR(ISP_ADDRL, 0xe4);
SFR(ISP_CMD, 0xe5);
SFR(ISP_TRIG, 0xe6);
SFR(ISP_CONTR, 0xe7);

/* System Clock Divider */
SFR(CLK_DIV, 0xc7);

/* I_O Port Mode Set Register */
SFR(P0M0, 0x93);
SFR(P0M1, 0x94);
SFR(P1M0, 0x91);
SFR(P1M1, 0x92);
SFR(P2M0, 0x95);
SFR(P2M1, 0x96);
SFR(P3M0, 0xb1);
SFR(P3M1, 0xb2);

/* SPI Register */
SFR(SPSTAT, 0x84);
SFR(SPCTL, 0x85);
SFR(SPDAT, 0x86);

/* ADC Register */
SFR(ADC_CONTR, 0xc5);
SFR(ADC_DATA, 0xc6);
SFR(ADC_LOW2, 0xbe);

/* PCA SFR */
SFR(CCON, 0xD8);
SFR(CMOD, 0xD9);
SFR(CCAPM0, 0xDA);
SFR(CCAPM1, 0xDB);
SFR(CCAPM2, 0xDC);
SFR(CCAPM3, 0xDD);
SFR(CCAPM4, 0xDE);
SFR(CCAPM5, 0xDF);

SFR(CL, 0xE9);
SFR(CCAP0L, 0xEA);
SFR(CCAP1L, 0xEB);
SFR(CCAP2L, 0xEC);
SFR(CCAP3L, 0xED);
SFR(CCAP4L, 0xEE);
SFR(CCAP5L, 0xEF);

SFR(CH, 0xF9);
SFR(CCAP0H, 0xFA);
SFR(CCAP1H, 0xFB);
SFR(CCAP2H, 0xFC);
SFR(CCAP3H, 0xFD);
SFR(CCAP4H, 0xFE);
SFR(CCAP5H, 0xFF);

SFR(PCA_PWM0, 0xF2);
SFR(PCA_PWM1, 0xF3);
SFR(PCA_PWM2, 0xF4);
SFR(PCA_PWM3, 0xF5);
SFR(PCA_PWM4, 0xF6);
SFR(PCA_PWM5, 0xF7);

/*  CCON  */
SBIT(CF, 0xDF);
SBIT(CR, 0xDE);
SBIT(CCF5, 0xDD);
SBIT(CCF4, 0xDC);
SBIT(CCF3, 0xDB);
SBIT(CCF2, 0xDA);
SBIT(CCF1, 0xD9);
SBIT(CCF0, 0xD8);


/* Above is STC additional SFR or change */

/*--------------------------------------------------------------------------
REG51F.H

Header file for 8xC31/51, 80C51Fx, 80C51Rx+
Copyright (c) 1988-1999 Keil Elektronik GmbH and Keil Software, Inc.
All rights reserved.

Modification according to DataSheet from April 1999
 - SFR's AUXR and AUXR1 added for 80C51Rx+ derivatives
--------------------------------------------------------------------------*/

/*  BYTE Registers  */
SFR(P0, 0x80);
SFR(P1, 0x90);
SFR(P2, 0xA0);
SFR(P3, 0xB0);
SFR(PSW, 0xD0);
SFR(ACC, 0xE0);
SFR(B, 0xF0);
SFR(SP, 0x81);
SFR(DPL, 0x82);
SFR(DPH, 0x83);
SFR(PCON, 0x87);
SFR(TCON, 0x88);
SFR(TMOD, 0x89);
SFR(TL0, 0x8A);
SFR(TL1, 0x8B);
SFR(TH0, 0x8C);
SFR(TH1, 0x8D);
SFR(IE, 0xA8);
SFR(IP, 0xB8);
SFR(SCON, 0x98);
SFR(SBUF, 0x99);

/*  80C51Fx/Rx Extensions  */
SFR(AUXR, 0x8E);
/* SFR(AUXR1, 0xA2); */
SFR(SADDR, 0xA9);
SFR(IPH, 0xB7);
SFR(SADEN, 0xB9);
SFR(T2CON, 0xC8);
SFR(T2MOD, 0xC9);
SFR(RCAP2L, 0xCA);
SFR(RCAP2H, 0xCB);
SFR(TL2, 0xCC);
SFR(TH2, 0xCD);


/*  BIT Registers  */
/*  PSW   */
SBIT(CY, 0xd7);
SBIT(AC, 0xd6);
SBIT(F0, 0xd5);
SBIT(RS1, 0xd4);
SBIT(RS0, 0xd3);
SBIT(OV, 0xd2);
SBIT(P, 0xd0);

/*  TCON  */
SBIT(TF1, 0x8f);
SBIT(TR1, 0x8e);
SBIT(TF0, 0x8d);
SBIT(TR0, 0x8c);
SBIT(IE1, 0x8b);
SBIT(IT1, 0x8a);
SBIT(IE0, 0x89);
SBIT(IT0, 0x88);

/*  P3  */
SBIT(RD, 0xb7);
SBIT(WR, 0xb6);
SBIT(T1, 0xb5);
SBIT(T0, 0xb4);
SBIT(INT1, 0xb3);
SBIT(INT0, 0xb2);
SBIT(TXD, 0xb1);
SBIT(RXD, 0xb0);

/*  SCON  */
SBIT(SM0, 0x9f); // alternatively "FE"
SBIT(FE, 0x9f);
SBIT(SM1, 0x9e);
SBIT(SM2, 0x9d);
SBIT(REN, 0x9c);
SBIT(TB8, 0x9b);
SBIT(RB8, 0x9a);
SBIT(TI, 0x99);
SBIT(RI, 0x98);
             

SBIT(T2EX, 0x91);
SBIT(T2, 0x90);

/*  T2CON  */
SBIT(TF2, 0xcf);
SBIT(EXF2, 0xce);
SBIT(RCLK, 0xcd);
SBIT(TCLK, 0xcc);
SBIT(EXEN2, 0xcb);
SBIT(TR2, 0xca);
SBIT(C_T2, 0xc9);
SBIT(CP_RL2, 0xc8);

/* PCA Pin */

SBIT(CEX3, 0xa4);
SBIT(CEX2, 0xa0);
SBIT(CEX1, 0xb5);
SBIT(CEX0, 0xb7);
SBIT(ECI, 0xb4);

/*  IE   */
SBIT(EA, 0xaf);
SBIT(EPCA_LVD, 0xae);
SBIT(EADC_SPI, 0xad);
SBIT(ES, 0xac);
SBIT(ET1, 0xab);
SBIT(EX1, 0xaa);
SBIT(ET0, 0xa9);
SBIT(EX0, 0xa8);

/*  IP   */ 
SBIT(PPCA_LVD, 0xbe);
SBIT(PADC_SPI, 0xbd); 
SBIT(PS, 0xbc);
SBIT(PT1, 0xbb);
SBIT(PX1, 0xba);
SBIT(PT0, 0xb9);
SBIT(PX0, 0xb8);

/////////////////////////////////////////////////

#endif

