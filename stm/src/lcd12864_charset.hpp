#ifndef __LCD12864_CHARSET_H__
#define __LCD12864_CHARSET_H__

/**
 * Character set for the 128x64 graphical LCD
 * 
 * This header file defines a bunch of extern const Images,
 * which encode characters. To implement a character set, provide
 * concrete values for these extern variables. See font.cpp for an
 * example.
 * 
 * Characters marked with "Unused" can be left unimplemented.
 */

#include <stdint.h>

// The lower bound of the "special" (non-ASCII) charset (inclusive).
#define LCD_CHARSET_LOWBOUND 0x0a
// The upper bound of the "special" (non-ASCII) charset (inclusive).
#define LCD_CHARSET_HIGHBOUND 0x1F

// The greek letter mu character in a string.
#define LCD_STR_MU "\x0a"
// The right arrow character in a string.
#define LCD_STR_RARW "\x0b"
// The negation (NOT) character in a string.
#define LCD_STR_LNOT "\x0c"
// The exclusive disjunction (XOR) character in a string.
#define LCD_STR_LXOR "\x0d"
// The logical (inclusive) disjunction (OR) character in a string.
#define LCD_STR_LOR "\x0e"
// The logical conjunction (AND) character in a string.
#define LCD_STR_LAND "\x0f"
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
// The character for math error in a string.
#define LCD_STR_MERR "\xff"

// The greek letter mu character.
#define LCD_CHAR_MU '\x0a'
// The right arrow character.
#define LCD_CHAR_RARW '\x0b'
// The negation (NOT) character.
#define LCD_CHAR_LNOT '\x0c'
// The exclusive disjunction (XOR) character.
#define LCD_CHAR_LXOR '\x0d'
// The logical (inclusive) disjunction (OR) character.
#define LCD_CHAR_LOR '\x0e'
// The logical conjunction (AND) character.
#define LCD_CHAR_LAND '\x0f'
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
// The character for math error.
#define LCD_CHAR_MERR '\xff'

/* This part not needed right now
// Verify that plain char is unsigned
// This must be true for the small charset to work
#include <limits.h>
#if CHAR_MIN < 0
    #error "Plain char must be unsigned!"
#endif
*/

namespace lcd {

    class Image;

    // Space ( )
	extern const Image CHAR_SPC;
    // Exclamation mark (!)
	extern const Image CHAR_EXMK;
    // Double quotes (")
    // Unused
	extern const Image CHAR_DBLQ;
    // Number sign (#)
    // Unused
	extern const Image CHAR_NSGN;
    // Dollar sign ($)
    // Unused
	extern const Image CHAR_DLR;
    // Percent sign (%)
	extern const Image CHAR_PCT;
    // Ampersand (&)
	extern const Image CHAR_AND;
    // Single quote (')
    // Unused
	extern const Image CHAR_SGLQ;
    // Left bracket (()
	extern const Image CHAR_LBKT;
    // Right bracket ())
	extern const Image CHAR_RBKT;
    // Asterisk (*)
	extern const Image CHAR_ASTK;
    // Plus (+)
	extern const Image CHAR_PLUS;
    // Comma (,)
	extern const Image CHAR_CMMA;
    // Minus (-)
	extern const Image CHAR_MNUS;
    // Dot (.)
	extern const Image CHAR_DOT;
    // Slash (/)
    // Unused
	extern const Image CHAR_SLSH;
    // 0 (0)
	extern const Image CHAR_0;
    // 1 (1)
	extern const Image CHAR_1;
    // 2 (2)
	extern const Image CHAR_2;
    // 3 (3)
	extern const Image CHAR_3;
    // 4 (4)
	extern const Image CHAR_4;
    // 5 (5)
	extern const Image CHAR_5;
    // 6 (6)
	extern const Image CHAR_6;
    // 7 (7)
	extern const Image CHAR_7;
    // 8 (8)
	extern const Image CHAR_8;
    // 9 (9)
	extern const Image CHAR_9;
    // Colon (:)
	extern const Image CHAR_CLN;
    // Semicolon (;)
    // Unused
	extern const Image CHAR_SMCN;
    // Less than (<)
	extern const Image CHAR_LSTN;
    // Equal (=)
	extern const Image CHAR_EQL;
    // Greater than (>)
	extern const Image CHAR_GTTN;
    // Question mark (?)
	extern const Image CHAR_QNMK;
    // At symbol (@)
    // Unused
	extern const Image CHAR_ATSB;
    // Uppercase A (A)
	extern const Image CHAR_A;
    // Uppercase B (B)
	extern const Image CHAR_B;
    // Uppercase C (C)
	extern const Image CHAR_C;
    // Uppercase D (D)
	extern const Image CHAR_D;
    // Uppercase E (E)
	extern const Image CHAR_E;
    // Uppercase F (F)
	extern const Image CHAR_F;
    // Uppercase G (G)
	extern const Image CHAR_G;
    // Uppercase H (H)
	extern const Image CHAR_H;
    // Uppercase I (I)
	extern const Image CHAR_I;
    // Uppercase J (J)
	extern const Image CHAR_J;
    // Uppercase K (K)
	extern const Image CHAR_K;
    // Uppercase L (L)
	extern const Image CHAR_L;
    // Uppercase M (M)
	extern const Image CHAR_M;
    // Uppercase N (N)
	extern const Image CHAR_N;
    // Uppercase O (O)
	extern const Image CHAR_O;
    // Uppercase P (P)
	extern const Image CHAR_P;
    // Uppercase Q (Q)
	extern const Image CHAR_Q;
    // Uppercase R (R)
	extern const Image CHAR_R;
    // Uppercase S (S)
	extern const Image CHAR_S;
    // Uppercase T (T)
	extern const Image CHAR_T;
    // Uppercase U (U)
	extern const Image CHAR_U;
    // Uppercase V (V)
	extern const Image CHAR_V;
    // Uppercase W (W)
	extern const Image CHAR_W;
    // Uppercase X (X)
	extern const Image CHAR_X;
    // Uppercase Y (Y)
	extern const Image CHAR_Y;
    // Uppercase Z (Z)
	extern const Image CHAR_Z;
    // Left square bracket ([)
	extern const Image CHAR_LSQB;
    // Backslash (\)
    // Unused
	extern const Image CHAR_BKSL;
    // Right square bracket (])
	extern const Image CHAR_RSQB;
    // Caret (^)
	extern const Image CHAR_CART;
    // Underscore (_)
	extern const Image CHAR_USCR;
    // Backtick (`)
    // Unused
	extern const Image CHAR_BKTK;
	// Lowercase A (a)
	extern const Image CHAR_LCA;
	// Lowercase B (b)
	extern const Image CHAR_LCB;
	// Lowercase C (c)
	extern const Image CHAR_LCC;
	// Lowercase D (d)
	extern const Image CHAR_LCD;
	// Lowercase E (e)
	extern const Image CHAR_LCE;
	// Lowercase F (f)
	extern const Image CHAR_LCF;
	// Lowercase G (g)
	extern const Image CHAR_LCG;
	// Lowercase H (h)
	extern const Image CHAR_LCH;
	// Lowercase I (i)
	extern const Image CHAR_LCI;
	// Lowercase J (j)
	extern const Image CHAR_LCJ;
	// Lowercase K (k)
	extern const Image CHAR_LCK;
	// Lowercase L (l)
	extern const Image CHAR_LCL;
	// Lowercase M (m)
	extern const Image CHAR_LCM;
	// Lowercase N (n)
	extern const Image CHAR_LCN;
	// Lowercase O (o)
	extern const Image CHAR_LCO;
	// Lowercase P (p)
	extern const Image CHAR_LCP;
	// Lowercase Q (q)
	extern const Image CHAR_LCQ;
	// Lowercase R (r)
	extern const Image CHAR_LCR;
	// Lowercase S (s)
	extern const Image CHAR_LCS;
	// Lowercase T (t)
	extern const Image CHAR_LCT;
	// Lowercase U (u)
	extern const Image CHAR_LCU;
	// Lowercase V (v)
	extern const Image CHAR_LCV;
	// Lowercase W (w)
	extern const Image CHAR_LCW;
	// Lowercase X (x)
	extern const Image CHAR_LCX;
	// Lowercase Y (y)
	extern const Image CHAR_LCY;
	// Lowercase Z (z)
	extern const Image CHAR_LCZ;
    // Left curvy bracket ({)
    // Unused
	extern const Image CHAR_LCVB;
    // Pipe/vertical bar (|)
	extern const Image CHAR_PIPE;
    // Right curvy bracket (})
    // Unused
	extern const Image CHAR_RCVB;
    // Tilde (~)
    // Unused
	extern const Image CHAR_TLDE;
    // Unknown character (?)
	extern const Image CHAR_UNKNOWN;
	
    // Mu
    extern const Image CHAR_MU;
    // Right arrow
    extern const Image CHAR_RARW;
    // Logical NOT
    extern const Image CHAR_LNOT;
    // Logical XOR
    extern const Image CHAR_LXOR;
    // Logical OR
    extern const Image CHAR_LOR;
    // Logical AND
    extern const Image CHAR_LAND;
    // Empty checkbox
    extern const Image CHAR_ECB;
    // Checked checkbox
    extern const Image CHAR_CCB;
    // Less than or equal sign
    extern const Image CHAR_LEQ;
    // Greater than or equal sign
    extern const Image CHAR_GEQ;
    // Cross product (x)
	extern const Image CHAR_CRS;
    // Standard gravity (g0)
	extern const Image CHAR_AGV;
    // Avogadro's number (Na)
	extern const Image CHAR_AVGO;
    // Elementary charge (e-)
	extern const Image CHAR_ECHG;
    // Velocity of light (c)
	extern const Image CHAR_VLIG;
    // Times 10 to the power of (E)
	extern const Image CHAR_EE;
    // Euler's number (e)
	extern const Image CHAR_EULR;
    // Theta
	extern const Image CHAR_THETA;
    // Syntax error
	extern const Image CHAR_SERR;
    // Pi
	extern const Image CHAR_PI;
    // Multiplication sign (dot)
	extern const Image CHAR_MUL;
    // Division sign
	extern const Image CHAR_DIV;
	// Summation symbol (capital sigma)
	extern const Image CHAR_SUMMATION;
    // Product symbol (capital pi)
	extern const Image CHAR_PRODUCT;
	
	extern const Image * const CHAR_ASCII[];
	
	uint8_t asciiToIndex(char);
	const Image& getChar(char);

    /* SMALL CHARSET */
    // 0
    extern const Image CHAR_SMALL_0;
    // 1
    extern const Image CHAR_SMALL_1;
    // 2
    extern const Image CHAR_SMALL_2;
    // 3
    extern const Image CHAR_SMALL_3;
    // 4
    extern const Image CHAR_SMALL_4;
    // 5
    extern const Image CHAR_SMALL_5;
    // 6
    extern const Image CHAR_SMALL_6;
    // 7
    extern const Image CHAR_SMALL_7;
    // 8
    extern const Image CHAR_SMALL_8;
    // 9
    extern const Image CHAR_SMALL_9;

    // A
    extern const Image CHAR_SMALL_A;
    // B
    extern const Image CHAR_SMALL_B;
    // C
    extern const Image CHAR_SMALL_C;
    // D
    extern const Image CHAR_SMALL_D;
    // E
    extern const Image CHAR_SMALL_E;
    // F
    extern const Image CHAR_SMALL_F;
    // G
    extern const Image CHAR_SMALL_G;
    // H
    extern const Image CHAR_SMALL_H;
    // I
    extern const Image CHAR_SMALL_I;
    // J
    extern const Image CHAR_SMALL_J;
    // K
    extern const Image CHAR_SMALL_K;
    // L
    extern const Image CHAR_SMALL_L;
    // M
    extern const Image CHAR_SMALL_M;
    // N
    extern const Image CHAR_SMALL_N;
    // O
    extern const Image CHAR_SMALL_O;
    // P
    extern const Image CHAR_SMALL_P;
    // Q
    extern const Image CHAR_SMALL_Q;
    // R
    extern const Image CHAR_SMALL_R;
    // S
    extern const Image CHAR_SMALL_S;
    // T
    extern const Image CHAR_SMALL_T;
    // U
    extern const Image CHAR_SMALL_U;
    // V
    extern const Image CHAR_SMALL_V;
    // W
    extern const Image CHAR_SMALL_W;
    // X
    extern const Image CHAR_SMALL_X;
    // Y
    extern const Image CHAR_SMALL_Y;
    // Z
    extern const Image CHAR_SMALL_Z;

    // Dot (.)
    extern const Image CHAR_SMALL_DOT;
    // Equals sign (=)
    extern const Image CHAR_SMALL_EQL;
    // "10 to the power of" symbol
    extern const Image CHAR_SMALL_EE;
    // Minus sign (-)
    extern const Image CHAR_SMALL_MINUS;
    // Comma (,)
    extern const Image CHAR_SMALL_COMMA;

    extern const Image * const CHAR_SMALL_NUMS[];
    extern const Image * const CHAR_SMALL_LETTERS[];

    const Image& getSmallNumber(uint8_t);
    const Image& getSmallChar(char);

    // Periodic table outline
    extern const Image IMG_PTABLE;
}

#endif
