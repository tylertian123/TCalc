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
}

#endif
