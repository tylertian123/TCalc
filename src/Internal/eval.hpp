#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"
#include "dynamarr.hpp"

namespace eval {

    enum class TokenType : uint8_t {
        NUMBER,
        OPERATOR,
        L_BRACKET,
        R_BRACKET,
        FUNCTION,
    };

    class Token {
    public:
        virtual TokenType getType() = 0;
    };

    class Number : public Token {
    public:
        Number(double value) : value(value) {}
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

        uint8_t getPrecedence() const;

        virtual TokenType getType() override {
            return TokenType::OPERATOR;
        }

        static Operator* fromChar(char);
        //Because there are only a set number of possible operators, we can keep singletons
        static Operator OP_PLUS, OP_MINUS, OP_MULTIPLY, OP_DIVIDE, OP_EXPONENT;
    
    private:
        Operator(Type type) : type(type) {}
    };

    class LeftBracket : public Token {
    public:
        virtual TokenType getType() override {
            return TokenType::L_BRACKET;
        }
        
        static LeftBracket INSTANCE;
    
    private:
        //As with operators, left and right brackets are singletons
        LeftBracket() {}
    };
    class RightBracket : public Token {
    public:
        virtual TokenType getType() override {
            return TokenType::R_BRACKET;
        }

        static RightBracket INSTANCE;
    
    private:
        RightBracket() {}
    };

    //For now, functions only take one argument
    //Multi-arg functions might be added in the future.
    class Function : public Token {
    public:
        enum class Type : uint8_t {
            SIN, COS, TAN, ASIN, ACOS, ATAN
        };

        virtual TokenType getType() override {
            return TokenType::FUNCTION;
        }

    };

    DynamicArray<Token*, 4>* tokensFromExpr(neda::Container*);
}

#endif
