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
    DynamicArray<Token*>* tokensFromExpr(neda::Container *expr) {
        DynamicArray<Token*, 4> *arr = new DynamicArray<Token*, 4>();
        //Deref the result so the syntax won't be so awkward
        auto &exprs = *expr->getContents();
        uint16_t index = 0;

        while(index < exprs.length()) {
            switch(exprs[index]->getType()) {
            //If we encounter nested containers, just do a recursive call
            case neda::ObjType::CONTAINER:
            {
                arr->merge(tokensFromExpr((neda::Container*) exprs[index]));
                ++index;
                break;
            }
            case neda::ObjType::CHAR_TYPE: 
            {
                char ch = ((neda::Character*) exprs[index])->ch;
                if(ch == ' ') {
                    ++index;
                    break;
                }
                Operator *op = Operator::fromChar(ch);
                //Check if the character is an operator
                if(op) {
                    arr->add(op);
                    ++index;
                    break;
                }
                //Skip if the character is neither an operator or a digit
                if(!isDigit(ch)) {
                    ++index;
                    break;
                }
                //Find the end of the number
                uint16_t end = index;
                for(; exprIsDigit(exprs[index]) && end < exprs.length(); index ++);
                //+1 for null terminator
                char *numStr = new char[end - index + 1];
                for(uint16_t i = index; i < end; i ++) {
                    numStr[i - index] = extractChar(exprs[i]);
                }
                //Add null terminator
                numStr[end - index - 1] = '\0';
                //Convert to double
                double d = atof(numStr);
                arr->add(new Number(d));
                ++index;
                break;
            }
            default: break;
            }
        }
    }
}
