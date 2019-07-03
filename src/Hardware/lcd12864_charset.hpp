#ifndef __LCD12864_CHARSET_H__
#define __LCD12864_CHARSET_H__
#include "lcd12864.hpp"

#define LCD_CHARSET_LOWBOUND 0x14
#define LCD_CHARSET_HIGHBOUND 0x1F

#define LCD_STR_CRS "\x14"
#define LCD_STR_AGV "\x15"
#define LCD_STR_AVGO "\x16"
#define LCD_STR_ECHG "\x17"
#define LCD_STR_VLIG "\x18"
#define LCD_STR_EE "\x19"
#define LCD_STR_EULR "\x1a"
#define LCD_STR_THETA "\x1b"
#define LCD_STR_SERR "\x1c"
#define LCD_STR_PI "\x1d"
#define LCD_STR_MUL "\x1e"
#define LCD_STR_DIV "\x1f"

#define LCD_CHAR_CRS '\x14'
#define LCD_CHAR_AGV '\x15'
#define LCD_CHAR_AVGO '\x16'
#define LCD_CHAR_ECHG '\x17'
#define LCD_CHAR_VLIG '\x18'
#define LCD_CHAR_EE '\x19'
#define LCD_CHAR_EULR '\x1a'
#define LCD_CHAR_THETA '\x1b'
#define LCD_CHAR_SERR '\x1c'
#define LCD_CHAR_PI '\x1d'
#define LCD_CHAR_MUL '\x1e'
#define LCD_CHAR_DIV '\x1f'

namespace lcd {

	typedef LCD12864Image Img;

    extern const Img CHAR_SPC;
	extern const Img CHAR_EXMK;
	// extern const Img CHAR_DBLQ;
	// extern const Img CHAR_NSGN;
	// extern const Img CHAR_DLR;
	extern const Img CHAR_PCT;
	// extern const Img CHAR_AND;
	// extern const Img CHAR_SGLQ;
	extern const Img CHAR_LBKT;
	extern const Img CHAR_RBKT;
	// extern const Img CHAR_ASTK;
	extern const Img CHAR_PLUS;
	extern const Img CHAR_CMMA;
	extern const Img CHAR_MNUS;
	extern const Img CHAR_DOT;
	// extern const Img CHAR_SLSH;
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
	// extern const Img CHAR_SMCN;
	extern const Img CHAR_LSTN;
	extern const Img CHAR_EQL;
	extern const Img CHAR_GTTN;
	// extern const Img CHAR_QNMK;
	// extern const Img CHAR_ATSB;
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
    
    extern const Img CHAR_CRS;
    extern const Img CHAR_AGV;
    extern const Img CHAR_AVGO;
    extern const Img CHAR_ECHG;
    extern const Img CHAR_VLIG;
    extern const Img CHAR_EE;
    extern const Img CHAR_EULR;
    extern const Img CHAR_THETA;
	extern const Img CHAR_SERR;
	extern const Img CHAR_PI;
	extern const Img CHAR_MUL;
	extern const Img CHAR_DIV;
	
	extern const Img CHAR_SUMMATION;
	extern const Img CHAR_PRODUCT;
	
    extern const Img * const CHAR_ASCII[];
	
	uint8_t asciiToIndex(char);
	const Img& getChar(char);
}

#endif
