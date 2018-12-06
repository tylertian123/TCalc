#include "eval_o.hpp"

namespace eval_o {
    uint8_t opPrec(TokenData t) {
        switch(t) {
        case TokenType::OP_SPMULT:
        case TokenType::OP_SPDIV:
            return 0;
        case TokenType::OP_POW:
            return 1;
        case TokenType::OP_MULT:
        case TokenType::OP_DIV:
            return 2;
        case TokenType::OP_PLUS:
        case TokenType::OP_MINUS:
            return 3;
        default: return 0xFF;
        }
    }
    TokenData chToOp(char ch) {
        switch(ch) {
        case '+':
            return TokenType::OP_PLUS;

        case '-':
            return TokenType::OP_MINUS;

        case LCD_CHAR_MUL:
        case '*':
            return TokenType::OP_MULT;

        case LCD_CHAR_DIV:
        case '/':
            return TokenType::OP_DIV;

        default: return TokenType::ERR;
        }
    }	
}
