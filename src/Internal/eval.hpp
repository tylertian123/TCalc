#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"
#include "dynamarr.hpp"
#include "deque.hpp"
#include "util.hpp"
#include <math.h>

namespace eval {

    /*
     * Base Token class and type enum
     */
    enum class TokenType : uint8_t {
        NUMBER,
        FRACTION,
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
    
    class Fraction : public Token {
    public:
        Fraction(int64_t num, int64_t denom) : num(num), denom(denom) {
            reduce();
        }
        int64_t num;
        int64_t denom;

        virtual TokenType getType() override {
            return TokenType::FRACTION;
        }

        static int64_t gcd(int64_t, int64_t);
        static int64_t lcm(int64_t, int64_t);

        double doubleVal() const;
        bool isInteger() const;
        void reduce();

        Fraction& operator+=(const Fraction&);
        Fraction& operator-=(const Fraction&);
        Fraction& operator*=(const Fraction&);
        Fraction& operator/=(const Fraction&);

        bool pow(const Fraction&);
    };

    class Operator : public Token {
    public:
        enum class Type {
            PLUS, MINUS, MULTIPLY, DIVIDE, EXPONENT, 
            //Special multiplication and division
            //These operators have the highest precedence
            SP_MULT, SP_DIV,
        };

        Type type;

        uint8_t getPrecedence() const;

        virtual TokenType getType() override {
            return TokenType::OPERATOR;
        }

        static Operator* fromChar(char);
        //Because there are only a set number of possible operators, we can keep singletons
        static Operator OP_PLUS, OP_MINUS, OP_MULTIPLY, OP_DIVIDE, OP_EXPONENT, OP_SP_MULT, OP_SP_DIV;

        double operate(double, double);
        //Returns whether the operation was successful (in the case of fractional exponentiation)
        //Ugly, I know.
        bool operateOn(Fraction*, Fraction*);

        //Operates on two numericals, taking into account fractions and everything
        //The returned numerical is allocated on the heap and needs to be freed
        //The input is deleted
        Token* operate(Token*, Token*);
    
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
            SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, ASINH, ACOSH, ATANH, LN, LOG10, LOG2,
        };
        Function(Type type) : type(type) {}

        Type type;

        virtual TokenType getType() override {
            return TokenType::FUNCTION;
        }

        static Function* fromString(const char*);
        double compute(double);
    };

    //This will delete the Deque of tokens properly. It will destory all tokens in the array.
    template <uint16_t Increase>
    void freeTokens(Deque<Token*, Increase> *q) {
        while (!q->isEmpty()) {
            Token *t = q->dequeue();
            if (t->getType() == TokenType::NUMERICAL || t->getType() == TokenType::FUNCTION) {
                delete t;
            }
        }
    }
    template <uint16_t Increase>
    void freeTokens(DynamicArray<Token*, Increase> *q) {
        for(Token *t : *q) {
            if (t->getType() == TokenType::NUMERICAL || t->getType() == TokenType::FUNCTION) {
                delete t;
            }
        }
    }
    template <uint16_t Increase>
    void freeNumericals(Deque<Token*, Increase> *q) {
        while (!q->isEmpty()) {
            delete q->dequeue();
        }
    }
    bool isDigit(char);
    bool isNameChar(char);
    char extractChar(neda::NEDAObj*);
    int8_t compareNumericals(const Token*, const Token*);

    //Shunting yard algorithm
    //Note: This does not delete the tokens in the DynamicArray
    //Note: The tokens are shared between the DynamicArray and the result
    template <uint16_t Increase>
    Deque<Token*>* toPostfix(DynamicArray<Token*, Increase>* tokens) {
        Deque<Token*> *output = new Deque<Token*>();
        Deque<Token*> *opStack = new Deque<Token*>();
        for(Token *token : *tokens) {
            //Add to the output queue if the token is a numerical type
            if(token->getType() == TokenType::NUMERICAL) {
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

    //Note: Deletes the input
    template <uint16_t Increase>
    Numerical* evalPostfix(Deque<Token*, Increase>* expr) {
        Deque<Numerical*> stack;

        while(!expr->isEmpty()) {
            Token *token = expr->dequeue();
            //Directly push any numerical types onto the stack
            if(token->getType() == TokenType::NUMERICAL) {
                stack.push((Numerical*) token);
                continue;
            }
            else if(token->getType() == TokenType::FUNCTION) {
                //Syntax error: Not Enough Arguments
                if(stack.isEmpty()) {
                    //Do cleanup and return nullptr
                    delete token;
                    freeTokens(expr);
                    delete expr;
                    freeNumericals(&stack);
                    return nullptr;
                }
                else {
                    //Compute the function
                    //Keep a pointer to the numerical to compute with to avoid mem leaks
                    //Make sure to delete the function token after
                    Numerical *n = stack.pop();
                    Function *func = (Function*) token;
                    if(n->getNumericalType() == NumericalType::NUM) {
                        stack.push(new Number(func->compute(((Number*) n)->value)));
                    }
                    //Calling any function on a fraction converts it into a double
                    else {
                        stack.push(new Number(func->compute(((Fraction*) n)->doubleVal())));
                    }
                    delete n;
                    delete func;
                }
            }
            else if(token->getType() == TokenType::OPERATOR) {
                //Syntax error: Not enough numbers
                if(stack.length() < 2) {
                    //Do cleanup and return nullptr
                    freeTokens(expr);
                    delete expr;
                    freeNumericals(&stack);
                    return nullptr;
                }
                Operator *op = (Operator*) token;
                Numerical *rhs = stack.pop();
                Numerical *lhs = stack.pop();
				//Operators do not need to be freed
				stack.push(op->operate(lhs, rhs));
            }
        }

        delete expr;
        if(stack.length() != 1) {
            //Syntax error: Not enough operations or numbers
            while(!stack.isEmpty()) {
                delete stack.pop();
            }
            return nullptr;
        }
        
        return stack.pop();
    }
	
    Token* evaluate(neda::Container *expr, uint8_t varc = 0, const char **varn = nullptr, Token **varv = nullptr);
    Token* evaluate(DynamicArray<neda::NEDAObj*>*, uint8_t varc = 0, const char **varn = nullptr, Token **varv = nullptr);
}

#endif
