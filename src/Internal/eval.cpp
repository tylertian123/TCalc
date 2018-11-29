#include "eval.hpp"
#include "lcd12864_charset.hpp"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CONST_PI 3.14159265358979323846
#define CONST_E  2.71828182845904523536

namespace eval {

    /******************** Number ********************/
    Number* Number::constFromString(const char* str) {
        if(strcmp(str, LCD_STR_PI) == 0) {
            return new Number(CONST_PI);
        }
        else if(strcmp(str, LCD_STR_EULR) == 0) {
            return new Number(CONST_E);
        }
        else {
            return nullptr;
        }
    }

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
    double Operator::operate(double lhs, double rhs) {
        switch(type) {
        case Type::PLUS:
        {
            return lhs + rhs;
        }
        case Type::MINUS:
        {
            return lhs - rhs;
        }
        case Type::MULTIPLY:
        {
            return lhs * rhs;
        }
        case Type::DIVIDE:
        {
            return lhs / rhs;
        }
        case Type::EXPONENT:
        {
            return pow(lhs, rhs);
        }
        default: return NAN;
        }
    }

    /******************** LeftBracket ********************/
    LeftBracket LeftBracket::INSTANCE;

    /******************** RightBracket ********************/
    RightBracket RightBracket::INSTANCE;

    /******************** Function ********************/
    Function* Function::fromString(const char *str) {
        if(strcmp(str, "sin") == 0) {
            return new Function(Type::SIN);
        }
        else if(strcmp(str, "cos") == 0) {
            return new Function(Type::COS);
        }
        else if(strcmp(str, "tan") == 0) {
            return new Function(Type::TAN);
        }
        else if(strcmp(str, "asin") == 0) {
            return new Function(Type::ASIN);
        }
        else if(strcmp(str, "acos") == 0) {
            return new Function(Type::ACOS);
        }
        else if(strcmp(str, "atan") == 0) {
            return new Function(Type::ATAN);
        }
        else {
            return nullptr;
        }
    }
    double Function::compute(double arg) {
        switch(type) {
        case Type::SIN:
        {
            return sin(arg);
        }
        case Type::COS:
        {
            return cos(arg);
        }
        case Type::TAN:
        {
            return tan(arg);
        }
        case Type::ASIN:
        {
            return asin(arg);
        }
        case Type::ACOS:
        {
            return acos(arg);
        }
        case Type::ATAN:
        {
            return atan(arg);
        }

        default: return NAN;
        }
    }

    /******************** Other Functions ********************/
    bool isDigit(char ch) {
        return (ch >= '0' && ch <= '9') || ch == '.';
    }
    bool isNameChar(char ch) {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= LCD_CHARSET_LOWBOUND && ch <= LCD_CHARSET_HIGHBOUND);
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
        bool allowUnary = true;

        while (index < exprs.length()) {
            switch (exprs[index]->getType()) {
            //If we encounter nested containers, just do a recursive call
            case neda::ObjType::CONTAINER:
            {
                arr->merge(tokensFromExpr((neda::Container*) exprs[index]));
                ++index;
                allowUnary = false;
                break;
            }
            case neda::ObjType::L_BRACKET:
            {
                //If the last token was not an operator, then it must be an implied multiplication
                if(!allowUnary) {
                    arr->add(&Operator::OP_MULTIPLY);
                }
                arr->add(&LeftBracket::INSTANCE);
                ++index;
                //Allow unary operators right after open brackets
                allowUnary = true;
                break;
            }
            case neda::ObjType::R_BRACKET:
            {
                arr->add(&RightBracket::INSTANCE);
                ++index;
                //Unlike open brackets, unary operators are not allowed after close brackets
                allowUnary = false;
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
                allowUnary = false;
                break;
            }
            case neda::ObjType::SUPERSCRIPT:
            {
                arr->add(&Operator::OP_EXPONENT);
                arr->add(&LeftBracket::INSTANCE);
                arr->merge(tokensFromExpr((neda::Container*) ((neda::Superscript*) exprs[index])->getContents()));
                arr->add(&RightBracket::INSTANCE);

                ++index;
                allowUnary = false;
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
                    if(allowUnary && (op->type == Operator::Type::PLUS || op->type == Operator::Type::MINUS)) {
                        //If we do encounter a unary operator, translate it to multiplication
                        //This is so that the order of operations won't be messed up (namely exponentiation)
                        if(op->type == Operator::Type::MINUS) {
                            arr->add(new Number(-1));
                            arr->add(&Operator::OP_MULTIPLY);
                        }
                        ++index;
                        allowUnary = false;
                        break;
                    }
                    else {
                        arr->add(op);
                        ++index;
                        allowUnary = true;
                        break;
                    }
                }
                //The character is neither an operator nor a digit-must be either a function or variable
                if (!isDigit(ch)) {
                    //Find the end of this token
                    uint16_t end = index + 1;
                    //No need worrying about exprs[end] not being a char, since then extractChar will just return '\0'
                    for(; end < exprs.length() && isNameChar(extractChar(exprs[end])); end ++);
                    char *str = new char[end - index + 1];
                    for(uint16_t i = index; i < end; i ++) {
                        str[i - index] = extractChar(exprs[i]);
                    }
                    //Add null terminator
                    str[end - index] = '\0';
                    Function *func = Function::fromString(str);
                    //Add the function if it's valid
                    if(func) {
                        arr->add(func);
                        //Allow unary operators after functions
                        allowUnary = true;
                    }
                    //Otherwise see if it's a valid constant
                    else {
                        Number *n = Number::constFromString(str);
                        if(n) {
                            arr->add(n);
                        }
                        allowUnary = false;
                    }

                    index = end;
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
                allowUnary = false;
                break;
            }
            default: break;
            }
        }
        return arr;
    }
}
