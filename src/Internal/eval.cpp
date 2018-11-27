#include "eval.hpp"

namespace eval {

    /******************** Operator ********************/
    uint8_t Operator::getPrecedence() {
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
}
