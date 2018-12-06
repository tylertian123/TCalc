#ifndef __EVAL_O_H__
#define __EVAL_O_H__

#include "neda.hpp"
#include "dynamarr.hpp"
#include "deque.hpp"
#include "util.hpp"
#include "lcd12864_charset.hpp"
#include <math.h>

#define EVAL_ISOP(x) ((x) >= eval_o::TokenType::OP_SPMULT && (x) <= eval_o::TokenType::OP_POW)
#define EVAL_ISFUNC(x) ((x) >= eval_o::TokenType::F_SIN && (x) <= eval_o::TokenType::F_LOG2)

namespace eval_o {
	typedef uint8_t TokenData;
	enum TokenType : TokenData {
		//Operators
		//SPMULT and SPDIV are special, high precedence multiplication and division
		OP_SPMULT, OP_SPDIV, OP_PLUS, OP_MINUS, OP_MULT, OP_DIV, OP_POW,

		//Brackets
		LBRAC, RBRAC,

		//Functions
		F_SIN, F_COS, F_TAN, F_ASIN, F_ACOS, F_ATAN, F_SINH, F_COSH, F_TANH, F_ASINH, F_ACOSH,
		F_ATANH, F_LOGE, F_LOG10, F_LOG2,

		//Other
		NUM = 0xFD, FRAC = 0xFE, ERR = 0xFF,
	};

    uint8_t opPrec(TokenData);
    TokenData chToOp(char);
	
}

#endif

