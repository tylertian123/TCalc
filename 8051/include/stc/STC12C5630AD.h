#ifndef __STC12C5630AD_H_
#define __STC12C5630AD_H_

/////////////////////////////////////////////////

/* The following is STC additional SFR or change */

/* __sfr __at(0x8e) AUXR; */
/* __sfr __at(0xb7) IPH; */

/* Watchdog Timer Register */
__sfr __at(0xe1) WDT_CONTR;    

/* ISP_IAP_EEPROM Register */
__sfr __at(0xe2) ISP_DATA;
__sfr __at(0xe3) ISP_ADDRH;
__sfr __at(0xe4) ISP_ADDRL;
__sfr __at(0xe5) ISP_CMD;
__sfr __at(0xe6) ISP_TRIG;
__sfr __at(0xe7) ISP_CONTR;

/* System Clock Divider */
__sfr __at(0xc7) CLK_DIV;

/* I_O Port Mode Set Register */
__sfr __at(0x93) P0M0;
__sfr __at(0x94) P0M1;
__sfr __at(0x91) P1M0;
__sfr __at(0x92) P1M1;
__sfr __at(0x95) P2M0;
__sfr __at(0x96) P2M1;
__sfr __at(0xb1) P3M0;
__sfr __at(0xb2) P3M1;

/* SPI Register */
__sfr __at(0x84) SPSTAT;
__sfr __at(0x85) SPCTL;
__sfr __at(0x86) SPDAT;

/* ADC Register */
__sfr __at(0xc5) ADC_CONTR;
__sfr __at(0xc6) ADC_DATA;
__sfr __at(0xbe) ADC_LOW2;

/* PCA SFR */
__sfr __at(0xD8) CCON;
__sfr __at(0xD9) CMOD;
__sfr __at(0xDA) CCAPM0;
__sfr __at(0xDB) CCAPM1;
__sfr __at(0xDC) CCAPM2;
__sfr __at(0xDD) CCAPM3;
__sfr __at(0xDE) CCAPM4;
__sfr __at(0xDF) CCAPM5;

__sfr __at(0xE9) CL;
__sfr __at(0xEA) CCAP0L;
__sfr __at(0xEB) CCAP1L;
__sfr __at(0xEC) CCAP2L;
__sfr __at(0xED) CCAP3L;
__sfr __at(0xEE) CCAP4L;
__sfr __at(0xEF) CCAP5L;

__sfr __at(0xF9) CH;
__sfr __at(0xFA) CCAP0H;
__sfr __at(0xFB) CCAP1H;
__sfr __at(0xFC) CCAP2H;
__sfr __at(0xFD) CCAP3H;
__sfr __at(0xFE) CCAP4H;
__sfr __at(0xFF) CCAP5H;

__sfr __at(0xF2) PCA_PWM0;
__sfr __at(0xF3) PCA_PWM1;
__sfr __at(0xF4) PCA_PWM2;
__sfr __at(0xF5) PCA_PWM3;
__sfr __at(0xF6) PCA_PWM4;
__sfr __at(0xF7) PCA_PWM5;

/*  CCON  */
__sbit __at(0xDF) CF;
__sbit __at(0xDE) CR;
__sbit __at(0xDD) CCF5;
__sbit __at(0xDC) CCF4;
__sbit __at(0xDB) CCF3;
__sbit __at(0xDA) CCF2;
__sbit __at(0xD9) CCF1;
__sbit __at(0xD8) CCF0;


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
__sfr __at(0x80) P0;
__sfr __at(0x90) P1;
__sfr __at(0xA0) P2;
__sfr __at(0xB0) P3;
__sfr __at(0xD0) PSW;
__sfr __at(0xE0) ACC;
__sfr __at(0xF0) B;
__sfr __at(0x81) SP;
__sfr __at(0x82) DPL;
__sfr __at(0x83) DPH;
__sfr __at(0x87) PCON;
__sfr __at(0x88) TCON;
__sfr __at(0x89) TMOD;
__sfr __at(0x8A) TL0;
__sfr __at(0x8B) TL1;
__sfr __at(0x8C) TH0;
__sfr __at(0x8D) TH1;
__sfr __at(0xA8) IE;
__sfr __at(0xB8) IP;
__sfr __at(0x98) SCON;
__sfr __at(0x99) SBUF;

/*  80C51Fx/Rx Extensions  */
__sfr __at(0x8E) AUXR;
/* __sfr __at(0xA2) AUXR1; */
__sfr __at(0xA9) SADDR;
__sfr __at(0xB7) IPH;
__sfr __at(0xB9) SADEN;
__sfr __at(0xC8) T2CON;
__sfr __at(0xC9) T2MOD;
__sfr __at(0xCA) RCAP2L;
__sfr __at(0xCB) RCAP2H;
__sfr __at(0xCC) TL2;
__sfr __at(0xCD) TH2;


/*  BIT Registers  */
/*  PSW   */
__sbit __at(0xd7) CY;
__sbit __at(0xd6) AC;
__sbit __at(0xd5) F0;
__sbit __at(0xd4) RS1;
__sbit __at(0xd3) RS0;
__sbit __at(0xd2) OV;
__sbit __at(0xd0) P;

/*  TCON  */
__sbit __at(0x8f) TF1;
__sbit __at(0x8e) TR1;
__sbit __at(0x8d) TF0;
__sbit __at(0x8c) TR0;
__sbit __at(0x8b) IE1;
__sbit __at(0x8a) IT1;
__sbit __at(0x89) IE0;
__sbit __at(0x88) IT0;

/*  P3  */
__sbit __at(0xb7) RD;
__sbit __at(0xb6) WR;
__sbit __at(0xb5) T1;
__sbit __at(0xb4) T0;
__sbit __at(0xb3) INT1;
__sbit __at(0xb2) INT0;
__sbit __at(0xb1) TXD;
__sbit __at(0xb0) RXD;

/*  SCON  */
__sbit __at(0x9f) SM0; // alternatively "FE"
__sbit __at(0x9f) FE;
__sbit __at(0x9e) SM1;
__sbit __at(0x9d) SM2;
__sbit __at(0x9c) REN;
__sbit __at(0x9b) TB8;
__sbit __at(0x9a) RB8;
__sbit __at(0x99) TI;
__sbit __at(0x98) RI;
             

__sbit __at(0x91) T2EX;
__sbit __at(0x90) T2;

/*  T2CON  */
__sbit __at(0xcf) TF2;
__sbit __at(0xce) EXF2;
__sbit __at(0xcd) RCLK;
__sbit __at(0xcc) TCLK;
__sbit __at(0xcb) EXEN2;
__sbit __at(0xca) TR2;
__sbit __at(0xc9) C_T2;
__sbit __at(0xc8) CP_RL2;

/* PCA Pin */

__sbit __at(0xa4) CEX3;
__sbit __at(0xa0) CEX2;
__sbit __at(0xb5) CEX1;
__sbit __at(0xb7) CEX0;
__sbit __at(0xb4) ECI;

/*  IE   */
__sbit __at(0xaf) EA;
__sbit __at(0xae) EPCA_LVD;
__sbit __at(0xad) EADC_SPI;
__sbit __at(0xac) ES;
__sbit __at(0xab) ET1;
__sbit __at(0xaa) EX1;
__sbit __at(0xa9) ET0;
__sbit __at(0xa8) EX0;

/*  IP   */ 
__sbit __at(0xbe) PPCA_LVD;
__sbit __at(0xbd) PADC_SPI; 
__sbit __at(0xbc) PS;
__sbit __at(0xbb) PT1;
__sbit __at(0xba) PX1;
__sbit __at(0xb9) PT0;
__sbit __at(0xb8) PX0;

/////////////////////////////////////////////////

#endif

