#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"

namespace eval {

    enum class TokenType : uint8_t {
        NUMBER,
        OPERATOR,
        L_BRACKET,
        R_BRACKET,
    };

    class Token {
    public:
        virtual TokenType getType() = 0;
    };

    class Number : public Token {
    public:
        double value;

        virtual TokenType getType() override {
            return TokenType::NUMBER;
        }
    };

    class Operator : public Token {
    public:

        enum class Type {
            PLUS, MINUS, MULTIPLY, DIVIDE, EXPONENT,
        };

        Type type;

        uint8_t getPrecedence();

        virtual TokenType getType() override {
            return TokenType::NUMBER;
        }
    };

    class LeftBracket : public Token {
    public:
        virtual TokenType getType() override {
            return TokenType::L_BRACKET;
        }
    };
    class RightBracket : public Token {
    public:
        virtual TokenType getType() override {
            return TokenType::R_BRACKET;
        }
    };
}

#endif
