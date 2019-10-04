#include "lcd12864_charset.hpp"

namespace lcd {
	const Image * const CHAR_ASCII[] = {
        &CHAR_MU,
        &CHAR_RARW,
        &CHAR_LNOT,
        &CHAR_LXOR,
        &CHAR_LOR,
        &CHAR_LAND,
        &CHAR_ECB,
        &CHAR_CCB,
        &CHAR_LEQ,
        &CHAR_GEQ,
		&CHAR_CRS,
		&CHAR_AGV,
		&CHAR_AVGO,
		&CHAR_ECHG,
		&CHAR_VLIG,
		&CHAR_EE,
		&CHAR_EULR,
		&CHAR_THETA,
		&CHAR_SERR,
		&CHAR_PI,
		&CHAR_MUL, 
		&CHAR_DIV,
		&CHAR_SPC,
		&CHAR_EXMK,
		&CHAR_UNKNOWN,//&CHAR_DBLQ,
		&CHAR_UNKNOWN,//&CHAR_NSGN,
		&CHAR_UNKNOWN,//&CHAR_DLR,
		&CHAR_PCT,
		&CHAR_UNKNOWN,//&CHAR_AND,
		&CHAR_UNKNOWN,//&CHAR_SGLQ,
		&CHAR_LBKT,
		&CHAR_RBKT,
		&CHAR_ASTK,
		&CHAR_PLUS,
		&CHAR_CMMA,
		&CHAR_MNUS,
		&CHAR_DOT,
		&CHAR_UNKNOWN,//&CHAR_SLSH,
		&CHAR_0,
		&CHAR_1,
		&CHAR_2,
		&CHAR_3,
		&CHAR_4,
		&CHAR_5,
		&CHAR_6,
		&CHAR_7,
		&CHAR_8,
		&CHAR_9,
		&CHAR_CLN,
		&CHAR_UNKNOWN,//&CHAR_SMCN,
		&CHAR_LSTN,
		&CHAR_EQL,
		&CHAR_GTTN,
		&CHAR_QNMK,
		&CHAR_UNKNOWN,//&CHAR_ATSB,
		&CHAR_A,
		&CHAR_B,
		&CHAR_C,
		&CHAR_D,
		&CHAR_E,
		&CHAR_F,
		&CHAR_G,
		&CHAR_H,
		&CHAR_I,
		&CHAR_J,
		&CHAR_K,
		&CHAR_L,
		&CHAR_M,
		&CHAR_N,
		&CHAR_O,
		&CHAR_P,
		&CHAR_Q,
		&CHAR_R,
		&CHAR_S,
		&CHAR_T,
		&CHAR_U,
		&CHAR_V,
		&CHAR_W,
		&CHAR_X,
		&CHAR_Y,
		&CHAR_Z,
		&CHAR_UNKNOWN,//&CHAR_LSQB,
		&CHAR_UNKNOWN,//&CHAR_BKSL,
		&CHAR_UNKNOWN,//&CHAR_RSQB,
		&CHAR_CART,
		&CHAR_UNKNOWN,//&CHAR_USCR,
		&CHAR_UNKNOWN,//&CHAR_BKTK,
		&CHAR_LCA,
		&CHAR_LCB,
		&CHAR_LCC,
		&CHAR_LCD,
		&CHAR_LCE,
		&CHAR_LCF,
		&CHAR_LCG,
		&CHAR_LCH,
		&CHAR_LCI,
		&CHAR_LCJ,
		&CHAR_LCK,
		&CHAR_LCL,
		&CHAR_LCM,
		&CHAR_LCN,
		&CHAR_LCO,
		&CHAR_LCP,
		&CHAR_LCQ,
		&CHAR_LCR,
		&CHAR_LCS,
		&CHAR_LCT,
		&CHAR_LCU,
		&CHAR_LCV,
		&CHAR_LCW,
		&CHAR_LCX,
		&CHAR_LCY,
		&CHAR_LCZ,
		&CHAR_UNKNOWN,//&CHAR_LCVB,
		&CHAR_PIPE,
		&CHAR_UNKNOWN,//&CHAR_RCVB,
		&CHAR_UNKNOWN,//&CHAR_TLDE,
		&CHAR_UNKNOWN,
	};
	
	uint8_t asciiToIndex(char c) {
		// If char is outside the range, return 0x7F (CHAR_UNKNOWN) minus the offset
		if(c < LCD_CHARSET_LOWBOUND || c >= 0x7F) {
			return 0x7F - LCD_CHARSET_LOWBOUND;
		}
		// Otherwise subtract the offset
		return c - LCD_CHARSET_LOWBOUND;
	}
	const Image& getChar(char c) {
        // Special processing for small charset
		return *CHAR_ASCII[asciiToIndex(c)];
	}

    const Image * const CHAR_SMALL_NUMS[] = {
        &CHAR_SMALL_0,
        &CHAR_SMALL_1,
        &CHAR_SMALL_2,
        &CHAR_SMALL_3,
        &CHAR_SMALL_4,
        &CHAR_SMALL_5,
        &CHAR_SMALL_6,
        &CHAR_SMALL_7,
        &CHAR_SMALL_8,
        &CHAR_SMALL_9,
    };

    const Image * const CHAR_SMALL_LETTERS[] = {
        &CHAR_SMALL_A,
        &CHAR_SMALL_B,
        &CHAR_SMALL_C,
        &CHAR_SMALL_D,
        &CHAR_SMALL_E,
        &CHAR_SMALL_F,
        &CHAR_SMALL_G,
        &CHAR_SMALL_H,
        &CHAR_SMALL_I,
        &CHAR_SMALL_J,
        &CHAR_SMALL_K,
        &CHAR_SMALL_L,
        &CHAR_SMALL_M,
        &CHAR_SMALL_N,
        &CHAR_SMALL_O,
        &CHAR_SMALL_P,
        &CHAR_SMALL_Q,
        &CHAR_SMALL_R,
        &CHAR_SMALL_S,
        &CHAR_SMALL_T,
        &CHAR_SMALL_U,
        &CHAR_SMALL_V,
        &CHAR_SMALL_W,
        &CHAR_SMALL_X,
        &CHAR_SMALL_Y,
        &CHAR_SMALL_Z,
    };

    const Image& getSmallNumber(uint8_t digit) {
        return *CHAR_SMALL_NUMS[digit];
    }

    const Image& getSmallChar(char c) {
        if(c >= '0' && c <= '9') {
            return *CHAR_SMALL_NUMS[c - '0'];
        }
        else if(c >= 'a' && c <= 'z') {
            return *CHAR_SMALL_LETTERS[c - 'a'];
        }
        else if(c >= 'A' && c <= 'Z') {
            return *CHAR_SMALL_LETTERS[c - 'A'];
        }
        else {
            switch(c) {
            case '=':
                return CHAR_SMALL_EQL;
            case '.':
                return CHAR_SMALL_DOT;
            case '-':
                return CHAR_SMALL_MINUS;
			case ',':
				return CHAR_SMALL_COMMA;
            case LCD_CHAR_EE:
                return CHAR_SMALL_EE;
            default:
                return CHAR_UNKNOWN;
            }
        }
    }
}
