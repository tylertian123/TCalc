#include "eval.hpp"
#include "lcd12864_charset.hpp"
#include <stdlib.h>

namespace eval {

    /******************** Operator ********************/
    uint8_t Operator::getPrecedence() const {
        switch(type) {
        case Type::EXPONENT:
            return 0;
        case Type::MULTIPLY:
        case Type::DIVIDE:
            return 1;
        case Type::PLUS:
        case Type::MINUS:
            return 2;
        default: return 0xFF;
        }
    }
    Operator* Operator::fromChar(char ch) {
        switch(ch) {
        case '+':
            return &OP_PLUS;
            
        case '-':
            return &OP_MINUS;

        case LCD_CHAR_MUL:
        case '*':
            return &OP_MULTIPLY;

        case LCD_CHAR_DIV:
        case '/':
            return &OP_DIVIDE;

        case '^':
            return &OP_EXPONENT;

        default: return nullptr;
        }
    }
    Operator Operator::OP_PLUS = { Operator::Type::PLUS },
                   Operator::OP_MINUS = { Operator::Type::MINUS },
                   Operator::OP_MULTIPLY = { Operator::Type::MULTIPLY },
                   Operator::OP_DIVIDE = { Operator::Type::DIVIDE },
                   Operator::OP_EXPONENT = { Operator::Type::EXPONENT };

    /******************** LeftBracket ********************/
    LeftBracket LeftBracket::INSTANCE;

    /******************** RightBracket ********************/
    RightBracket RightBracket::INSTANCE;

    /******************** Functions ********************/
    bool isDigit(char ch) {
        return (ch >= '0' && ch <= '9') || ch == '.';
    }
    bool exprIsDigit(neda::NEDAObj *obj) {
        if(obj->getType() != neda::ObjType::CHAR_TYPE) {
            return false;
        }
        return isDigit(((neda::Character*) obj)->ch);
    }
    char extractChar(neda::NEDAObj *obj) {
        if(obj->getType() != neda::ObjType::CHAR_TYPE) {
            return '\0';
        }
        return ((neda::Character*) obj)->ch;
    }
    DynamicArray<Token*, 4>* tokensFromExpr(neda::Container *expr) {
        DynamicArray<Token*, 4> *arr = new DynamicArray<Token*, 4>();
        //Deref the result so the syntax won't be so awkward
        auto &exprs = *expr->getContents();
        uint16_t index = 0;
        //This variable keeps track of whether the last token was an operator
        //It is used for unary operators like the unary minus and plus
        bool lastTokenIsOperator = true;

        while (index < exprs.length()) {
            switch (exprs[index]->getType()) {
                //If we encounter nested containers, just do a recursive call
            case neda::ObjType::CONTAINER:
            {
                arr->merge(tokensFromExpr((neda::Container*) exprs[index]));
                ++index;
                lastTokenIsOperator = false;
                break;
            }
            case neda::ObjType::L_BRACKET:
            {
                //If the last token was not an operator, then it must be an implied multiplication
                arr->add(&Operator::OP_MULTIPLY);
                arr->add(&LeftBracket::INSTANCE);
                ++index;
                //Allow unary operators right after open brackets
                lastTokenIsOperator = true;
                break;
            }
            case neda::ObjType::R_BRACKET:
            {
                arr->add(&RightBracket::INSTANCE);
                ++index;
                //Unlike open brackets, unary operators are not allowed after close brackets
                lastTokenIsOperator = false;
                break;
            }
            case neda::ObjType::FRACTION:
            {
                //Translate fractions to a division with brackets
                arr->add(&LeftBracket::INSTANCE);
                //Merge this with the result of a token extraction on the numerator
                arr->merge(tokensFromExpr((neda::Container*) ((neda::Fraction*) exprs[index])->getNumerator()));
                arr->add(&RightBracket::INSTANCE);
                arr->add(&Operator::OP_DIVIDE);
                arr->add(&LeftBracket::INSTANCE);
                arr->merge(tokensFromExpr((neda::Container*) ((neda::Fraction*) exprs[index])->getDenominator()));
                arr->add(&RightBracket::INSTANCE);
                
                ++index;
                lastTokenIsOperator = false;
                break;
            }
            case neda::ObjType::SUPERSCRIPT:
            {
                arr->add(&Operator::OP_EXPONENT);
                arr->add(&LeftBracket::INSTANCE);
                arr->merge(tokensFromExpr((neda::Container*) ((neda::Superscript*) exprs[index])->getContents()));
                arr->add(&RightBracket::INSTANCE);

                ++index;
                lastTokenIsOperator = false;
                break;
            }
            case neda::ObjType::CHAR_TYPE:
            {
                char ch = ((neda::Character*) exprs[index])->ch;
                if (ch == ' ') {
                    ++index;
                    break;
                }
                Operator *op = Operator::fromChar(ch);
                //Check if the character is an operator
                if (op) {
                    //Check for unary operators
                    //Last token must be an operator
                    if(lastTokenIsOperator && (op->type == Operator::Type::PLUS || op->type == Operator::Type::MINUS)) {
                        //If we do encounter a unary operator, translate it to multiplication
                        //This is so that the order of operations won't be messed up (namely exponentiation)
                        if(op->type == Operator::Type::MINUS) {
                            arr->add(new Number(-1));
                            arr->add(&Operator::OP_MULTIPLY);
                        }
                        ++index;
                        lastTokenIsOperator = false;
                        break;
                    }
                    else {
                        arr->add(op);
                        ++index;
                        lastTokenIsOperator = true;
                        break;
                    }
                }
                //Skip if the character is neither an operator or a digit
                if (!isDigit(ch)) {
                    ++index;
                    break;
                }

                //Otherwise find the end of the number as usual
                uint16_t end = index + 1;
                for (; end < exprs.length() && exprIsDigit(exprs[end]); ++end);
                //+1 for null terminator
                char *numStr = new char[end - index + 1];
                for (uint16_t i = index; i < end; i++) {
                    numStr[i - index] = extractChar(exprs[i]);
                }
                //Add null terminator
                numStr[end - index] = '\0';
                //Convert to double
                double d = atof(numStr);
                arr->add(new Number(d));
                index = end;
                lastTokenIsOperator = false;
                break;
            }
            default: break;
            }
        }
        return arr;
    }
}
