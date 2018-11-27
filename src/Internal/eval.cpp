#include "eval.hpp"
#include "lcd12864_charset.hpp"

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
    const Operator* Operator::fromChar(char ch) {
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
            
        default: return nullptr;
        }
    }
    const Operator Operator::OP_PLUS = { Operator::Type::PLUS },
                   Operator::OP_MINUS = { Operator::Type::MINUS },
                   Operator::OP_MULTIPLY = { Operator::Type::MULTIPLY },
                   Operator::OP_DIVIDE = { Operator::Type::DIVIDE },
                   Operator::OP_EXPONENT = { Operator::Type::EXPONENT };

    /******************** Functions ********************/
    DynamicArray<Token*>* tokensFromExpr(neda::Container *expr) {
        DynamicArray<Token*> *arr = new DynamicArray<Token*>();
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
            case neda::ObjType::CHAR_TYPE: {

                break;
            }
            default: break;
            }
        }
    }
}
