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
__sbit __at(PSW^7) CY;
__sbit __at(PSW^6) AC;
__sbit __at(PSW^5) F0;
__sbit __at(PSW^4) RS1;
__sbit __at(PSW^3) RS0;
__sbit __at(PSW^2) OV;
__sbit __at(PSW^0) P;

/*  TCON  */
__sbit __at(TCON^7) TF1;
__sbit __at(TCON^6) TR1;
__sbit __at(TCON^5) TF0;
__sbit __at(TCON^4) TR0;
__sbit __at(TCON^3) IE1;
__sbit __at(TCON^2) IT1;
__sbit __at(TCON^1) IE0;
__sbit __at(TCON^0) IT0;

/*  P3  */
__sbit __at(P3^7) RD;
__sbit __at(P3^6) WR;
__sbit __at(P3^5) T1;
__sbit __at(P3^4) T0;
__sbit __at(P3^3) INT1;
__sbit __at(P3^2) INT0;
__sbit __at(P3^1) TXD;
__sbit __at(P3^0) RXD;

/*  SCON  */
__sbit __at(SCON^7) SM0; // alternatively "FE"
__sbit __at(SCON^7) FE;
__sbit __at(SCON^6) SM1;
__sbit __at(SCON^5) SM2;
__sbit __at(SCON^4) REN;
__sbit __at(SCON^3) TB8;
__sbit __at(SCON^2) RB8;
__sbit __at(SCON^1) TI;
__sbit __at(SCON^0) RI;
             

__sbit __at(P1^1) T2EX;
__sbit __at(P1^0) T2;

/*  T2CON  */
__sbit __at(T2CON^7) TF2;
__sbit __at(T2CON^6) EXF2;
__sbit __at(T2CON^5) RCLK;
__sbit __at(T2CON^4) TCLK;
__sbit __at(T2CON^3) EXEN2;
__sbit __at(T2CON^2) TR2;
__sbit __at(T2CON^1) C_T2;
sbit CP_RL2= T2CON^0;

/* PCA Pin */

__sbit __at(P2^4) CEX3;
__sbit __at(P2^0) CEX2;
__sbit __at(P3^5) CEX1;
__sbit __at(P3^7) CEX0;
__sbit __at(P3^4) ECI;

/*  IE   */
__sbit __at(IE^7) EA;
__sbit __at(IE^6) EPCA_LVD;
__sbit __at(IE^5) EADC_SPI;
__sbit __at(IE^4) ES;
__sbit __at(IE^3) ET1;
__sbit __at(IE^2) EX1;
__sbit __at(IE^1) ET0;
__sbit __at(IE^0) EX0;

/*  IP   */ 
__sbit __at(IP^6) PPCA_LVD;
__sbit __at(IP^5) PADC_SPI; 
__sbit __at(IP^4) PS;
__sbit __at(IP^3) PT1;
__sbit __at(IP^2) PX1;
__sbit __at(IP^1) PT0;
__sbit __at(IP^0) PX0;

/////////////////////////////////////////////////

#endif

