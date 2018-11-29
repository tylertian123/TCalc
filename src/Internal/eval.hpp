#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"
#include "dynamarr.hpp"
#include "deque.hpp"

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
        
        static Number* constFromString(const char*);
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
    //Even though only one instance of each type of function is needed, because there are a lot of functions, it is not worth it
    //to make it a singleton
    class Function : public Token {
    public:
        enum class Type : uint8_t {
            SIN, COS, TAN, ASIN, ACOS, ATAN
        };
        Function(Type type) : type(type) {}

        Type type;

        virtual TokenType getType() override {
            return TokenType::FUNCTION;
        }

        static Function* fromString(const char*);

    };

    DynamicArray<Token*, 4>* tokensFromExpr(neda::Container*);

    //Shunting yard algorithm
    //Note: This does not delete the tokens in the DynamicArray
    //Note: The tokens are shared between the DynamicArray and the result
    template <uint16_t Increase>
    Deque<Token*>* toPostfix(DynamicArray<Token*, Increase>* tokens) {
        Deque<Token*> *output = new Deque<Token*>();
        Deque<Token*> *opStack = new Deque<Token*>();
        for(Token *token : *tokens) {
            //Add to the output queue if the token is a number
            if(token->getType() == TokenType::NUMBER) {
                output->enqueue(token);
            }
            //Push directly onto the op stack in case of a function or left bracket
            else if(token->getType() == TokenType::L_BRACKET || token->getType() == TokenType::FUNCTION) {
                opStack->push(token);
            }
            else if(token->getType() == TokenType::OPERATOR) {
                //While the operator on top of the stack has equal or higher precedence, or the operator on top is a function,
                //pop them off the stack and put into the queue
                Token *op;
                while(!opStack->isEmpty() && (op = opStack->peek(), op->getType() == TokenType::FUNCTION || 
                        (op->getType() == TokenType::OPERATOR && ((Operator*) op)->getPrecedence() <= ((Operator*) token)->getPrecedence()))) {
                    output->enqueue(opStack->pop());
                }
                //Finally push the operator
                opStack->push(token);
            }
            else if(token->getType() == TokenType::R_BRACKET) {
                //While the operator on top of the stack is not a left bracket, pop items off the stack and into the queue
                Token *op;
                while(!opStack->isEmpty() && !(op = opStack->peek(), op->getType() == TokenType::L_BRACKET)) {
                    output->enqueue(opStack->pop());
                }
                //If everything is good, the stack should not be empty (as it still has a left bracket that's not popped off)
                if(!opStack->isEmpty()) {
                    opStack->pop();
                }
                //Otherwise there are unmatched brackets
                else {
                    //Do cleanup
                    //Note: the stack and queue are deleted, but the tokens are not!!
                    delete opStack;
                    delete output;
                    return nullptr;
                }
            }
        }
        //Pop everything on the stack into the queue
        while(!opStack->isEmpty()) {
            //Ignore left brackets
            if(opStack->peek()->getType() != TokenType::L_BRACKET) {
                output->enqueue(opStack->pop());
            }
            else {
                opStack->pop();
            }
        }
        delete opStack;
        return output;
    }

    //This will delete the DynamicArray of tokens properly. It will destory all tokens in the array and the array itself.
    template <uint16_t Increase>
    void freeTokens(DynamicArray<Token*, Increase> *arr) {
        for(Token *token : *arr) {
            //Only delete if token is not of a singleton class
            if(token->getType() == TokenType::NUMBER || token->getType() == TokenType::FUNCTION) {
                delete token;
            }
        }
        delete arr;
    }
}

#endif
