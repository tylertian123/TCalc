#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"
#include "dynamarr.hpp"
#include "deque.hpp"
#include "util.hpp"

namespace eval {

    /*
     * Base Token class and type enum
     */
    enum class TokenType : uint8_t {
        NUMERICAL,
        OPERATOR,
        L_BRACKET,
        R_BRACKET,
        FUNCTION,
    };
    class Token {
    public:
        virtual TokenType getType() = 0;
    };

    /*
     * Numerical class and type enum
     * In order to have both numbers and fractions, there is a base class shared between the Number and Fraction classes
     */
    enum class NumericalType : uint8_t {
        NUM,
        FRAC,
    };
    class Numerical : public Token {
    public:
        virtual TokenType getType() override {
            return TokenType::NUMERICAL;
        }
        
        virtual NumericalType getNumericalType() = 0;
    };

    class Number : public Numerical {
    public:
        Number(double value) : value(value) {}
        double value;

        virtual NumericalType getNumericalType() override {
            return NumericalType::NUM;
        }
        
        static Number* constFromString(const char*);
    };
    
    class Fraction : public Numerical {
    public:
        Fraction(int64_t num, int64_t denom) : num(num), denom(denom) {
            reduce();
        }
        int64_t num;
        int64_t denom;

        virtual NumericalType getNumericalType() override {
            return NumericalType::FRAC;
        }

        static int64_t gcd(int64_t, int64_t);
        static int64_t lcm(int64_t, int64_t);

        double doubleVal() const;
        bool isInteger() const;
        void reduce();

        Fraction operator+(const Fraction&);
        Fraction operator-(const Fraction&);
        Fraction operator*(const Fraction&);
        Fraction operator/(const Fraction&);
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
        //Returns whether the operation was successful (in the case of fractional exponentiation)
        //Ugly, I know.
        bool operateOn(Fraction*, Fraction*);
    
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
     * syntax error) and a pointer to a Numerical through a double pointer (Note: the value of the double pointer will not be changed
     * in the case of a syntax error). IT DELETES ITS INPUT. The input Deque, as well as all the Tokens pointed to by the pointers 
     * inside, will all be deleted, even in the case of a syntax error aborting the evaluation. The returned Numerical also needs to be
     * freed manually.
     * 
     * MEMORY-LEAK-FREE EVALUATION SEQUENCE
     * 
     * auto tokens = tokensFromExpr(expr);
     * //delete expr; //If necessary
     * auto postfixTokens = toPostfix(tokens);
     * delete tokens; //Delete the tokens array itself, not the contents
     * Numerical *n = nullptr; //This only serves as a dummy
     * Numerical *result = &n;
     * bool success = evalPostfix(postfixTokens, result);
     * //Do stuff with result...
     * if(success) {
     *     delete *result;
     * }
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
    bool evalPostfix(Deque<Token*, Increase>* expr, Numerical **out) {
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
                    //Do cleanup and return false
                    delete token;
                    freeTokens(expr);
                    return false;
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
                    //Do cleanup and return false
                    freeTokens(expr);
                    return false;
                }
                Operator *op = (Operator*) token;
                Numerical *rhs = stack.pop();
                Numerical *lhs = stack.pop();
                //Operators don't need to be freed
                //Do different operations based on the numerical type of the operands
                NumericalType lType = lhs->getNumericalType();
                NumericalType rType = rhs->getNumericalType();
                //Two numbers: normal operation
                if(lType == NumericalType::NUM && rType == NumericalType::NUM) {
                    //Special case for division: if the operands are whole numbers, create a fraction
                    if(op->type == Operator::Type::DIVIDE && isInt(((Number*) lhs)->value) && isInt(((Number*) rhs)->value)) {
                        stack.push(new Fraction(static_cast<int64_t>(((Number*) lhs)->value), static_cast<int64_t>(((Number*) rhs)->value)));
                    }
                    else {
                        stack.push(new Number(op->operate(((Number*) lhs)->value, ((Number*) rhs)->value)));
                    }
                    delete lhs;
                    delete rhs;
                }
                //Two fractions: fraction operation
                else if(lType == NumericalType::FRAC && rType == NumericalType::FRAC) {
                    //Record if action was successful
                    bool success = op->operateOn((Fraction*) lhs, (Fraction*) rhs);
                    if(success) {
                        stack.push(lhs);
                        delete rhs;
                    }
                    //If the operation was not possible, convert to double and operate normally
                    else {
                        stack.push(new Number(op->operate(((Fraction*) lhs)->doubleVal(), ((Fraction*) rhs)->doubleVal())));
                        delete lhs;
                        delete rhs;
                    }
                }
                //One fraction: fraction operation if the other one is integer, normal operation if not
                else if(lType == NumericalType::FRAC && rType == NumericalType::NUM) {
                    //Test if rhs is integer
                    if(isInt(((Number*) rhs)->value)) {
                        //Do a normal fraction operation
                        //Since the rhs is an integer, this operation is guaranteed to succeed
                        op->operateOn((Fraction*) lhs, &Fraction((uint64_t) ((Number*) rhs)->value, 1));
                        stack.push(lhs);
                        delete rhs;
                    }
                    //Otherwise convert to doubles
                    else {
                        stack.push(new Number(op->operate(((Fraction*) lhs)->doubleVal(), ((Number*) rhs)->value)));
                        delete lhs;
                        delete rhs;
                    }
                }
                else {
                    if(isInt(((Number*) lhs)->value)) {
                        //This operation is not guaranteed to succeed
                        //Construct fraction since it's not going to be temporary if this operation succeeds
                        Fraction *lhsFrac = new Fraction((uint64_t) ((Number*) lhs)->value, 1);
                        bool success = op->operateOn(lhsFrac, (Fraction*) rhs);
                        if(success) {
                            stack.push(lhsFrac);
                            delete lhs;
                            delete rhs;
                        }
                        else {
                            delete lhsFrac;
                            //My code is terrible, indeed.
                            goto convertToDoubleAndOperate;
                        }
                    }
                    else {
                    convertToDoubleAndOperate:
                        stack.push(new Number(op->operate(((Number*) lhs)->value, ((Fraction*) rhs)->doubleVal())));
                        delete lhs;
                        delete rhs;
                    }
                }
            }
        }

        delete expr;
        if(stack.length() != 1) {
            //Syntax error: Not enough operations or numbers
            while(!stack.isEmpty()) {
                delete stack.pop();
            }
            return false;
        }
        
        *out = stack.pop();
        return true;
    }

    //This will delete the DynamicArray of tokens properly. It will destory all tokens in the array and the array itself.
    template <uint16_t Increase>
    void freeTokens(DynamicArray<Token*, Increase> *arr) {
        for(Token *token : *arr) {
            //Only delete if token is not of a singleton class
            if(token->getType() == TokenType::NUMERICAL || token->getType() == TokenType::FUNCTION) {
                delete token;
            }
        }
        delete arr;
    }
    template <uint16_t Increase>
    void freeTokens(Deque<Token*, Increase> *q) {
        while(!q->isEmpty()) {
            Token *t = q->dequeue();
            if(t->getType() == TokenType::NUMERICAL || t->getType() == TokenType::FUNCTION) {
                delete t;
            }
        }
        delete q;
    }
}

#endif
