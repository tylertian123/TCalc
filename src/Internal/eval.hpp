#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"
#include "dynamarr.hpp"
#include "deque.hpp"

namespace eval {

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
        Fraction(int32_t num, int32_t denom) : num(num), denom(denom) {
            reduce();
        }
        int32_t num;
        int32_t denom;

        virtual TokenType getType() override {
            return TokenType::FRACTION;
        }

        static int32_t gcd(int32_t, int32_t);
        static int32_t lcm(int32_t, int32_t);

        double doubleVal();
        bool isInteger();
        void reduce();

        Fraction operator+(const Fraction&);
        Fraction operator-(const Fraction&);
        Fraction operator*(const Fraction&);
        Fraction operator/(const Fraction&);

        Fraction raiseToInt(uint32_t);
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

        double operate(double, double);
    
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
        double compute(double);
    };

    /*
     * MEMORY MANAGEMENT WITH tokensFromExpr, toPostFix AND evalPostfix
     * 
     * tokensFromExpr takes in a pointer to a neda::Container and returns a pointer to a DynamicArray of Token pointers.
     * It does not free or modify the input; thus the input NEDA object needs to be deleted manually.
     * The returned DynamicArray and all of its contents are allocated on the HEAP; therefore, the Array itself and all of its contents
     * need to be deleted manually.
     * 
     * toPostfix takes in a pointer to a DynamicArray of Token pointers, and returns a pointer to a Deque of Token pointers.
     * It does not free or modify the input; thus the input Array needs to be deleted manually.
     * The returned Deque is allocated on the HEAP, however, all of its contents are SHARED with that of the input DynamicArray.
     * In other words, toPostFix does not create new objects, but rather uses the existing ones. Therefore, if one of the input Array's
     * contents or the output Deque's contents are deleted, the other will also be affected. The DynamicArray and the Deque themselves
     * also need to be deleted manually.
     * 
     * evalPostfix takes in a pointer to a Deque of Token pointers, and returns a boolean (true if evaluation was successful, false if
     * syntax error) and outputs a double through a reference. IT DELETES ITS INPUT. The input Deque, as well as all the Tokens pointed
     * to by the pointers inside, will all be deleted, even in the case of a syntax error aborting the evaluation.
     * 
     * MEMORY-LEAK-FREE EVALUATION SEQUENCE
     * 
     * auto tokens = tokensFromExpr(expr);
     * //delete expr; //If necessary
     * auto postfixTokens = toPostfix(tokens);
     * delete tokens; //Delete the tokens array itself, not the contents
     * double result;
     * bool success = evalPostfix(postfixTokens, result);
     * 
     */
    DynamicArray<Token*, 4>* tokensFromExpr(neda::Container*);

    //Shunting yard algorithm
    //Note: This does not delete the tokens in the DynamicArray
    //Note: The tokens are shared between the DynamicArray and the result
    template <uint16_t Increase>
    Deque<Token*>* toPostfix(DynamicArray<Token*, Increase>* tokens) {
        Deque<Token*> *output = new Deque<Token*>();
        Deque<Token*> *opStack = new Deque<Token*>();
        for(Token *token : *tokens) {
            //Add to the output queue if the token is a number or fraction
            if(token->getType() == TokenType::NUMBER || TokenType::FRACTION) {
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
    bool evalPostfix(Deque<Token*, Increase>* expr, double &out) {
        Deque<double> stack;
        while(!expr->isEmpty()) {
            Token *token = expr->dequeue();
            if(token->getType() == TokenType::NUMBER) {
                stack.push(((Number*) token)->value);
                //Make sure the number is freed
                delete token;
                continue;
            }
            else if(token->getType() == TokenType::FUNCTION) {
                //Syntax error: Not Enough Arguments
                if(stack.isEmpty()) {
                    //Do cleanup and return false
                    delete token;
                    freeTokens(expr);
                    return false;
                }
                else {
                    //Compute the function
                    stack.push(((Function*) token)->compute(stack.pop()));
                }
            }
            else if(token->getType() == TokenType::OPERATOR) {
                //Syntax error: Not enough numbers
                if(stack.length() < 2) {
                    //Do cleanup and return false
                    delete token;
                    freeTokens(expr);
                    return false;
                }
                double rhs = stack.pop();
                double lhs = stack.pop();
                stack.push(((Operator*) token)->operate(lhs, rhs));
            }
        }

        delete expr;
        if(stack.length() != 1) {
            //Syntax error: Not enough operations or numbers
            return false;
        }
        
        out = stack.pop();
        return true;
    }

    //This will delete the DynamicArray of tokens properly. It will destory all tokens in the array and the array itself.
    template <uint16_t Increase>
    void freeTokens(DynamicArray<Token*, Increase> *arr) {
        for(Token *token : *arr) {
            //Only delete if token is not of a singleton class
            if(token->getType() == TokenType::NUMBER || token->getType() == TokenType::FUNCTION || token->getType() == TokenType::FRACTION) {
                delete token;
            }
        }
        delete arr;
    }
    template <uint16_t Increase>
    void freeTokens(Deque<Token*, Increase> *q) {
        while(!q->isEmpty()) {
            Token *t = q->dequeue();
            if(t->getType() == TokenType::NUMBER || t->getType() == TokenType::FUNCTION || token->getType() == TokenType::FRACTION) {
                delete t;
            }
        }
        delete q;
    }
}

#endif
