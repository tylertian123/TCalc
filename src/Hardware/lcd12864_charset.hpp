#ifndef __LCD12864_CHARSET_H__
#define __LCD12864_CHARSET_H__
#include "lcd12864.hpp"

#define LCD_CHARSET_LOWBOUND 0x1B
#define LCD_CHAR_THETA "\x1b"
#define LCD_CHAR_SERR "\x1c"
#define LCD_CHAR_PI "\x1d"
#define LCD_CHAR_MUL "\x1e"
#define LCD_CHAR_DIV "\x1f"

namespace lcd {

	typedef LCD12864Image Img;

    extern const Img CHAR_SPC;
	extern const Img CHAR_EXMK;
	extern const Img CHAR_DBLQ;
	extern const Img CHAR_NSGN;
	extern const Img CHAR_DLR;
	extern const Img CHAR_PCT;
	extern const Img CHAR_AND;
	extern const Img CHAR_SGLQ;
	extern const Img CHAR_LBKT;
	extern const Img CHAR_RBKT;
	extern const Img CHAR_ASTK;
	extern const Img CHAR_PLUS;
	extern const Img CHAR_CMMA;
	extern const Img CHAR_MNUS;
	extern const Img CHAR_DOT;
	extern const Img CHAR_SLSH;
	extern const Img CHAR_0;
	extern const Img CHAR_1;
	extern const Img CHAR_2;
	extern const Img CHAR_3;
	extern const Img CHAR_4;
	extern const Img CHAR_5;
	extern const Img CHAR_6;
	extern const Img CHAR_7;
	extern const Img CHAR_8;
	extern const Img CHAR_9;
	extern const Img CHAR_CLN;
	extern const Img CHAR_SMCN;
	extern const Img CHAR_LSTN;
	extern const Img CHAR_EQL;
	extern const Img CHAR_GTTN;
	extern const Img CHAR_QNMK;
	extern const Img CHAR_ATSB;
	extern const Img CHAR_A;
	extern const Img CHAR_B;
	extern const Img CHAR_C;
	extern const Img CHAR_D;
	extern const Img CHAR_E;
	extern const Img CHAR_F;
	extern const Img CHAR_G;
	extern const Img CHAR_H;
	extern const Img CHAR_I;
	extern const Img CHAR_J;
	extern const Img CHAR_K;
	extern const Img CHAR_L;
	extern const Img CHAR_M;
	extern const Img CHAR_N;
	extern const Img CHAR_O;
	extern const Img CHAR_P;
	extern const Img CHAR_Q;
	extern const Img CHAR_R;
	extern const Img CHAR_S;
	extern const Img CHAR_T;
	extern const Img CHAR_U;
	extern const Img CHAR_V;
	extern const Img CHAR_W;
	extern const Img CHAR_X;
	extern const Img CHAR_Y;
	extern const Img CHAR_Z;
	extern const Img CHAR_LSQB;
	extern const Img CHAR_BKSL;
	extern const Img CHAR_RSQB;
	extern const Img CHAR_CART;
	extern const Img CHAR_USCR;
	extern const Img CHAR_BKTK;
	extern const Img CHAR_LCA;
	extern const Img CHAR_LCB;
	extern const Img CHAR_LCC;
	extern const Img CHAR_LCD;
	extern const Img CHAR_LCE;
	extern const Img CHAR_LCF;
	extern const Img CHAR_LCG;
	extern const Img CHAR_LCH;
	extern const Img CHAR_LCI;
	extern const Img CHAR_LCJ;
	extern const Img CHAR_LCK;
	extern const Img CHAR_LCL;
	extern const Img CHAR_LCM;
	extern const Img CHAR_LCN;
	extern const Img CHAR_LCO;
	extern const Img CHAR_LCP;
	extern const Img CHAR_LCQ;
	extern const Img CHAR_LCR;
	extern const Img CHAR_LCS;
	extern const Img CHAR_LCT;
	extern const Img CHAR_LCU;
	extern const Img CHAR_LCV;
	extern const Img CHAR_LCW;
	extern const Img CHAR_LCX;
	extern const Img CHAR_LCY;
	extern const Img CHAR_LCZ;
	extern const Img CHAR_LCVB;
	extern const Img CHAR_PIPE;
	extern const Img CHAR_RCVB;
	extern const Img CHAR_TLDE;
	extern const Img CHAR_UNKNOWN;
	
    extern const Img CHAR_THETA;
	extern const Img CHAR_SERR;
	extern const Img CHAR_PI;
	extern const Img CHAR_MUL;
	extern const Img CHAR_DIV;
	
	extern const Img CHAR_SUMMATION;
	extern const Img CHAR_PRODUCT;

    const Img CHAR_NUMBERS[] = {
        CHAR_0,
        CHAR_1,
        CHAR_2,
        CHAR_3,
        CHAR_4,
        CHAR_5,
        CHAR_6,
        CHAR_7,
        CHAR_8,
        CHAR_9,
    };
	
    const Img CHAR_ASCII[] = {
        CHAR_THETA,
		CHAR_SERR,
		CHAR_PI,
		CHAR_MUL, 
		CHAR_DIV,
        CHAR_SPC,
        CHAR_EXMK,
        CHAR_DBLQ,
        CHAR_NSGN,
        CHAR_DLR,
        CHAR_PCT,
        CHAR_AND,
        CHAR_SGLQ,
        CHAR_LBKT,
        CHAR_RBKT,
        CHAR_ASTK,
        CHAR_PLUS,
        CHAR_CMMA,
        CHAR_MNUS,
        CHAR_DOT,
        CHAR_SLSH,
        CHAR_0,
        CHAR_1,
        CHAR_2,
        CHAR_3,
        CHAR_4,
        CHAR_5,
        CHAR_6,
        CHAR_7,
        CHAR_8,
        CHAR_9,
        CHAR_CLN,
        CHAR_SMCN,
        CHAR_LSTN,
        CHAR_EQL,
        CHAR_GTTN,
        CHAR_QNMK,
        CHAR_ATSB,
        CHAR_A,
        CHAR_B,
        CHAR_C,
        CHAR_D,
        CHAR_E,
        CHAR_F,
        CHAR_G,
        CHAR_H,
        CHAR_I,
        CHAR_J,
        CHAR_K,
        CHAR_L,
        CHAR_M,
        CHAR_N,
        CHAR_O,
        CHAR_P,
        CHAR_Q,
        CHAR_R,
        CHAR_S,
        CHAR_T,
        CHAR_U,
        CHAR_V,
        CHAR_W,
        CHAR_X,
        CHAR_Y,
        CHAR_Z,
        CHAR_LSQB,
        CHAR_BKSL,
        CHAR_RSQB,
        CHAR_CART,
        CHAR_USCR,
        CHAR_BKTK,
        CHAR_LCA,
        CHAR_LCB,
        CHAR_LCC,
        CHAR_LCD,
        CHAR_LCE,
        CHAR_LCF,
        CHAR_LCG,
        CHAR_LCH,
        CHAR_LCI,
        CHAR_LCJ,
        CHAR_LCK,
        CHAR_LCL,
        CHAR_LCM,
        CHAR_LCN,
        CHAR_LCO,
        CHAR_LCP,
        CHAR_LCQ,
        CHAR_LCR,
        CHAR_LCS,
        CHAR_LCT,
        CHAR_LCU,
        CHAR_LCV,
        CHAR_LCW,
        CHAR_LCX,
        CHAR_LCY,
        CHAR_LCZ,
        CHAR_LCVB,
        CHAR_PIPE,
        CHAR_RCVB,
        CHAR_TLDE,
		CHAR_UNKNOWN,
    };
	
	uint8_t asciiToIndex(char);
	const Img& getChar(char);
	const Img& getNumber(Img);
}

#endif
