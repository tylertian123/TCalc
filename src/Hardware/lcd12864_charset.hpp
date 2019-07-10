#ifndef __LCD12864_CHARSET_H__
#define __LCD12864_CHARSET_H__

/**
 * Character set for the 128x64 graphical LCD
 * 
 * This header file defines a bunch of extern const LCD12864Images,
 * which encode characters. To implement a character set, provide
 * concrete values for these extern variables. See font.cpp for an
 * example.
 * 
 * Characters marked with "Unused" can be left unimplemented.
 */

#include "lcd12864.hpp"

// The lower bound of the "special" (non-ASCII) charset (inclusive).
#define LCD_CHARSET_LOWBOUND 0x10
// The upper bound of the "special" (non-ASCII) charset (inclusive).
#define LCD_CHARSET_HIGHBOUND 0x1F

// The empty checkbox character in a string.
#define LCD_STR_ECB "\x10"
// The checked checkbox character in a string.
#define LCD_STR_CCB "\x11"
// The less than or equal character in a string.
#define LCD_STR_LEQ "\x12"
// The greater than or equal to character in a string.
#define LCD_STR_GEQ "\x13"
// The cross product character (x) in a string.
#define LCD_STR_CRS "\x14"
// The character for standard gravity (g0) in a string.
#define LCD_STR_AGV "\x15"
// The character for Avogadro's number (Na) in a string.
#define LCD_STR_AVGO "\x16"
// The character for elementary charge (e-) in a string.
#define LCD_STR_ECHG "\x17"
// The character for the velocity of light (c) in a string.
#define LCD_STR_VLIG "\x18"
// The "10 to the power of" character (E) in a string.
#define LCD_STR_EE "\x19"
// The character for Euler's number (e) in a string.
#define LCD_STR_EULR "\x1a"
// The character theta in a string.
#define LCD_STR_THETA "\x1b"
// The character for syntax error in a string.
#define LCD_STR_SERR "\x1c"
// The character for pi in a string.
#define LCD_STR_PI "\x1d"
// The character for the multiplication symbol (dot) in a string.
#define LCD_STR_MUL "\x1e"
// The character for the division sign in a string.
#define LCD_STR_DIV "\x1f"

// The empty checkbox character.
#define LCD_CHAR_ECB '\x10'
// The checked checkbox character.
#define LCD_CHAR_CCB '\x11'
// The less than or equal character.
#define LCD_CHAR_LEQ '\x12'
// The greater than or equal to character.
#define LCD_CHAR_GEQ '\x13'
// The cross product character (x).
#define LCD_CHAR_CRS '\x14'
// The character for standard gravity (g0).
#define LCD_CHAR_AGV '\x15'
// The character for Avogadro's number (Na).
#define LCD_CHAR_AVGO '\x16'
// The character for elementary charge (e-).
#define LCD_CHAR_ECHG '\x17'
// The character for the velocity of light (c).
#define LCD_CHAR_VLIG '\x18'
// The "10 to the power of" character (E).
#define LCD_CHAR_EE '\x19'
// The character for Euler's number (e).
#define LCD_CHAR_EULR '\x1a'
// The character theta.
#define LCD_CHAR_THETA '\x1b'
// The character for syntax error.
#define LCD_CHAR_SERR '\x1c'
// The character for pi.
#define LCD_CHAR_PI '\x1d'
// The character for the multiplication symbol (dot).
#define LCD_CHAR_MUL '\x1e'
// The character for the division sign.
#define LCD_CHAR_DIV '\x1f'

namespace lcd {

	typedef LCD12864Image Img;

    // Space ( )
	extern const Img CHAR_SPC;
    // Exclamation mark (!)
	extern const Img CHAR_EXMK;
    // Double quotes (")
    // Unused
	extern const Img CHAR_DBLQ;
    // Number sign (#)
    // Unused
	extern const Img CHAR_NSGN;
    // Dollar sign ($)
    // Unused
	extern const Img CHAR_DLR;
    // Percent sign (%)
	extern const Img CHAR_PCT;
    // Ampersand (&)
    // Unused
	extern const Img CHAR_AND;
    // Single quote (')
    // Unused
	extern const Img CHAR_SGLQ;
    // Left bracket (()
	extern const Img CHAR_LBKT;
    // Right bracket ())
	extern const Img CHAR_RBKT;
    // Asterisk (*)
    // Unused
	extern const Img CHAR_ASTK;
    // Plus (+)
	extern const Img CHAR_PLUS;
    // Comma (,)
	extern const Img CHAR_CMMA;
    // Minus (-)
	extern const Img CHAR_MNUS;
    // Dot (.)
	extern const Img CHAR_DOT;
    // Slash (/)
    // Unused
	extern const Img CHAR_SLSH;
    // 0 (0)
	extern const Img CHAR_0;
    // 1 (1)
	extern const Img CHAR_1;
    // 2 (2)
	extern const Img CHAR_2;
    // 3 (3)
	extern const Img CHAR_3;
    // 4 (4)
	extern const Img CHAR_4;
    // 5 (5)
	extern const Img CHAR_5;
    // 6 (6)
	extern const Img CHAR_6;
    // 7 (7)
	extern const Img CHAR_7;
    // 8 (8)
	extern const Img CHAR_8;
    // 9 (9)
	extern const Img CHAR_9;
    // Colon (:)
	extern const Img CHAR_CLN;
    // Semicolon (;)
    // Unused
	extern const Img CHAR_SMCN;
    // Less than (<)
	extern const Img CHAR_LSTN;
    // Equal (=)
	extern const Img CHAR_EQL;
    // Greater than (>)
	extern const Img CHAR_GTTN;
    // Question mark (?)
    // Unused
	extern const Img CHAR_QNMK;
    // At symbol (@)
    // Unused
	extern const Img CHAR_ATSB;
    // Uppercase A (A)
	extern const Img CHAR_A;
    // Uppercase B (B)
	extern const Img CHAR_B;
    // Uppercase C (C)
	extern const Img CHAR_C;
    // Uppercase D (D)
	extern const Img CHAR_D;
    // Uppercase E (E)
	extern const Img CHAR_E;
    // Uppercase F (F)
	extern const Img CHAR_F;
    // Uppercase G (G)
	extern const Img CHAR_G;
    // Uppercase H (H)
	extern const Img CHAR_H;
    // Uppercase I (I)
	extern const Img CHAR_I;
    // Uppercase J (J)
	extern const Img CHAR_J;
    // Uppercase K (K)
	extern const Img CHAR_K;
    // Uppercase L (L)
	extern const Img CHAR_L;
    // Uppercase M (M)
	extern const Img CHAR_M;
    // Uppercase N (N)
	extern const Img CHAR_N;
    // Uppercase O (O)
	extern const Img CHAR_O;
    // Uppercase P (P)
	extern const Img CHAR_P;
    // Uppercase Q (Q)
	extern const Img CHAR_Q;
    // Uppercase R (R)
	extern const Img CHAR_R;
    // Uppercase S (S)
	extern const Img CHAR_S;
    // Uppercase T (T)
	extern const Img CHAR_T;
    // Uppercase U (U)
	extern const Img CHAR_U;
    // Uppercase V (V)
	extern const Img CHAR_V;
    // Uppercase W (W)
	extern const Img CHAR_W;
    // Uppercase X (X)
	extern const Img CHAR_X;
    // Uppercase Y (Y)
	extern const Img CHAR_Y;
    // Uppercase Z (Z)
	extern const Img CHAR_Z;
    // Left square bracket ([)
    // Unused
	extern const Img CHAR_LSQB;
    // Backslash (\)
    // Unused
	extern const Img CHAR_BKSL;
    // Right square bracket (])
    // Unused
	extern const Img CHAR_RSQB;
    // Caret (^)
    // Unused
	extern const Img CHAR_CART;
    // Underscore (_)
    // Unused
	extern const Img CHAR_USCR;
    // Backtick (`)
    // Unused
	extern const Img CHAR_BKTK;
	// Lowercase A (a)
	extern const Img CHAR_LCA;
	// Lowercase B (b)
	extern const Img CHAR_LCB;
	// Lowercase C (c)
	extern const Img CHAR_LCC;
	// Lowercase D (d)
	extern const Img CHAR_LCD;
	// Lowercase E (e)
	extern const Img CHAR_LCE;
	// Lowercase F (f)
	extern const Img CHAR_LCF;
	// Lowercase G (g)
	extern const Img CHAR_LCG;
	// Lowercase H (h)
	extern const Img CHAR_LCH;
	// Lowercase I (i)
	extern const Img CHAR_LCI;
	// Lowercase J (j)
	extern const Img CHAR_LCJ;
	// Lowercase K (k)
	extern const Img CHAR_LCK;
	// Lowercase L (l)
	extern const Img CHAR_LCL;
	// Lowercase M (m)
	extern const Img CHAR_LCM;
	// Lowercase N (n)
	extern const Img CHAR_LCN;
	// Lowercase O (o)
	extern const Img CHAR_LCO;
	// Lowercase P (p)
	extern const Img CHAR_LCP;
	// Lowercase Q (q)
	extern const Img CHAR_LCQ;
	// Lowercase R (r)
	extern const Img CHAR_LCR;
	// Lowercase S (s)
	extern const Img CHAR_LCS;
	// Lowercase T (t)
	extern const Img CHAR_LCT;
	// Lowercase U (u)
	extern const Img CHAR_LCU;
	// Lowercase V (v)
	extern const Img CHAR_LCV;
	// Lowercase W (w)
	extern const Img CHAR_LCW;
	// Lowercase X (x)
	extern const Img CHAR_LCX;
	// Lowercase Y (y)
	extern const Img CHAR_LCY;
	// Lowercase Z (z)
	extern const Img CHAR_LCZ;
    // Left curvy bracket ({)
    // Unused
	extern const Img CHAR_LCVB;
    // Pipe/vertical bar (|)
    // Unused
	extern const Img CHAR_PIPE;
    // Right curvy bracket (})
    // Unused
	extern const Img CHAR_RCVB;
    // Tilde (~)
    // Unused
	extern const Img CHAR_TLDE;
    // Unknown character (?)
	extern const Img CHAR_UNKNOWN;
	
    // Empty checkbox
    extern const Img CHAR_ECB;
    // Checked checkbox
    extern const Img CHAR_CCB;
    // Less than or equal sign
    extern const Img CHAR_LEQ;
    // Greater than or equal sign
    extern const Img CHAR_GEQ;
    // Cross product (x)
	extern const Img CHAR_CRS;
    // Standard gravity (g0)
	extern const Img CHAR_AGV;
    // Avogadro's number (Na)
	extern const Img CHAR_AVGO;
    // Elementary charge (e-)
	extern const Img CHAR_ECHG;
    // Velocity of light (c)
	extern const Img CHAR_VLIG;
    // Times 10 to the power of (E)
	extern const Img CHAR_EE;
    // Euler's number (e)
	extern const Img CHAR_EULR;
    // Theta
	extern const Img CHAR_THETA;
    // Syntax error
	extern const Img CHAR_SERR;
    // Pi
	extern const Img CHAR_PI;
    // Multiplication sign (dot)
	extern const Img CHAR_MUL;
    // Division sign
	extern const Img CHAR_DIV;
	// Summation symbol (capital sigma)
	extern const Img CHAR_SUMMATION;
    // Product symbol (capital pi)
	extern const Img CHAR_PRODUCT;
	
	extern const Img * const CHAR_ASCII[];
	
	uint8_t asciiToIndex(char);
	const Img& getChar(char);
}

#endif
