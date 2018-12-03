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

    /******************** Fraction ********************/
    int64_t Fraction::gcd(int64_t a, int64_t b) {
        while(true) {
            if(a == 0) {
                return b;
            }
            if(b == 0) {
                return a;
            }

            int64_t r = a % b;
            a = b;
            b = r;
        }
    }
    int64_t Fraction::lcm(int64_t a, int64_t b) {
        return (a * b) / gcd(a, b);
    }
    double Fraction::doubleVal() const {
        //Make sure they're cast to doubles first to avoid integer division
        return static_cast<double>(num) / static_cast<double>(denom);
    }
    bool Fraction::isInteger() const {
        return num % denom == 0;
    }
    void Fraction::reduce() {
        //Make sure the denominator is always positive
        if(denom < 0) {
            num *= -1;
            denom *= -1;
        }

        //Now that the denominator is positive, we can make sure the result we get is also positive
        int64_t divisor = gcd(num, denom);
        if(divisor == 1) {
            return;
        }
        num /= divisor;
        denom /= divisor;
    }
    Fraction Fraction::operator+(const Fraction &rhs) {
        int64_t newDenom = lcm(denom, rhs.denom);
        int64_t numA = num * (newDenom / denom);
        int64_t numB = rhs.num * (newDenom / rhs.denom);
        
        return Fraction(numA + numB, newDenom);
    }
    Fraction Fraction::operator-(const Fraction &rhs) {
        int64_t newDenom = lcm(denom, rhs.denom);
        int64_t numA = num * (newDenom / denom);
        int64_t numB = rhs.num * (newDenom / rhs.denom);

        return Fraction(numA - numB, newDenom);
    }
    Fraction Fraction::operator*(const Fraction &rhs) {
        return Fraction(num * rhs.num, denom * rhs.denom);
    }
    Fraction Fraction::operator/(const Fraction &rhs) {
        return Fraction(num * rhs.denom, denom * rhs.num);
    }
    Fraction& Fraction::operator+=(const Fraction &frac) {
        int64_t newDenom = lcm(denom, frac.denom);
        int64_t numA = num * (newDenom / denom);
        int64_t numB = frac.num * (newDenom / frac.denom);
        num = numA + numB;
        denom = newDenom;
        return *this;
    }
    Fraction& Fraction::operator-=(const Fraction &frac) {
        int64_t newDenom = lcm(denom, frac.denom);
        int64_t numA = num * (newDenom / denom);
        int64_t numB = frac.num * (newDenom / frac.denom);
        num = numA - numB;
        denom = newDenom;
        return *this;
    }
    Fraction& Fraction::operator*=(const Fraction &frac) {
        num *= frac.num;
        denom *= frac.denom;
        reduce();
        return *this;
    }
    Fraction& Fraction::operator/=(const Fraction &frac) {
        num *= frac.denom;
        denom *= frac.num;
        reduce();
        return *this;
    }
    bool Fraction::pow(const Fraction &other) {
        double e = other.doubleVal();
        double n = ::pow(num, e);
        //Check if the numerator and denominator are still ints
        if(!isInt(n)) {
            return false;
        }
        double d = ::pow(denom, e);
        if(!isInt(d)) {
            return false;
        }

        num = static_cast<int64_t>(n);
        denom = static_cast<int64_t>(d);
        reduce();
        return true;
    }

    /******************** Operator ********************/
    uint8_t Operator::getPrecedence() const {
        switch(type) {
        case Type::SP_MULT:
        case Type::SP_DIV:
            return 0;
        case Type::EXPONENT:
            return 1;
        case Type::MULTIPLY:
        case Type::DIVIDE:
            return 2;
        case Type::PLUS:
        case Type::MINUS:
            return 3;
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
             Operator::OP_EXPONENT = { Operator::Type::EXPONENT },
             Operator::OP_SP_MULT = { Operator::Type::SP_MULT },
             Operator::OP_SP_DIV = { Operator::Type::SP_DIV };
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
        case Type::SP_MULT:
        case Type::MULTIPLY:
        {
            return lhs * rhs;
        }
        case Type::SP_DIV:
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
    bool Operator::operateOn(Fraction *frac, Fraction *rhs) {
        switch(type) {
        case Type::PLUS:
        {
            *frac += *rhs;
            break;
        }
        case Type::MINUS:
        {
            *frac -= *rhs;
            break;
        }
        case Type::SP_MULT:
        case Type::MULTIPLY:
        {
            *frac *= *rhs;
            break;
        }
        case Type::SP_DIV:
        case Type::DIVIDE:
        {
            *frac /= *rhs;
            break;
        }
        case Type::EXPONENT:
        {
            return frac->pow(*rhs);
        }
        default: break;
        }
        return true;
    }
    Numerical* Operator::operate(Numerical *lhs, Numerical *rhs) {
        NumericalType lType = lhs->getNumericalType();
        NumericalType rType = rhs->getNumericalType();
        Numerical *result = nullptr;
        //Two numbers: normal operation
        if(lType == NumericalType::NUM && rType == NumericalType::NUM) {
            //Special case for division: if the operands are whole numbers, create a fraction
            if(type == Operator::Type::DIVIDE && isInt(((Number*) lhs)->value) && isInt(((Number*) rhs)->value)) {
                auto n = static_cast<int64_t>(((Number*) lhs)->value);
                auto d = static_cast<int64_t>(((Number*) rhs)->value);
                //See if the division yields a whole number
                if(n % d == 0) {
                    //If the result is an integer, just push the integer instead
                    result = new Number(n / d);
                }
                else {
                    //Otherwise create a fraction
                    result = new Fraction(n, d);
                }
            }
            else {
                result = new Number(operate(((Number*) lhs)->value, ((Number*) rhs)->value));
            }
            delete lhs;
            delete rhs;
        }
        //Two fractions: fraction operation
        else if(lType == NumericalType::FRAC && rType == NumericalType::FRAC) {
            //Record if action was successful
            bool success = operateOn((Fraction*) lhs, (Fraction*) rhs);
            if(success) {
                //See if result is an integer
                if(((Fraction*) lhs)->isInteger()) {
                    //If yes then directly insert a number
                    result = new Number(((Fraction*) lhs)->doubleVal());
                    delete lhs;
                }
                else {
                    result = lhs;
                }
                delete rhs;
            }
            //If the operation was not possible, convert to double and operate normally
            else {
                result = new Number(operate(((Fraction*) lhs)->doubleVal(), ((Fraction*) rhs)->doubleVal()));
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
                //Create this variable to avoid a compiler warning
                Fraction temp((uint64_t) ((Number*) rhs)->value, 1);
                operateOn((Fraction*) lhs, &temp);

                //Test if resulting fraction is an integer
                if(((Fraction*) lhs)->isInteger()) {
                    result = new Number(((Fraction*) lhs)->doubleVal());
                    delete lhs;
                }
                else {
                    result = lhs;
                }
                delete rhs;
            }
            //Otherwise convert to doubles
            else {
                result = new Number(operate(((Fraction*) lhs)->doubleVal(), ((Number*) rhs)->value));
                delete lhs;
                delete rhs;
            }
        }
        else {
            if(isInt(((Number*) lhs)->value)) {
                //This operation is not guaranteed to succeed
                //Construct fraction since it's not going to be temporary if this operation succeeds
                Fraction *lhsFrac = new Fraction((uint64_t) ((Number*) lhs)->value, 1);
                bool success = operateOn(lhsFrac, (Fraction*) rhs);
                if(success) {
                    if(lhsFrac->isInteger()) {
                        result = new Number(lhsFrac->doubleVal());
                        delete lhsFrac;
                    }
                    else {
                        result = lhsFrac;
                    }
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
                result = new Number(operate(((Number*) lhs)->value, ((Fraction*) rhs)->doubleVal()));
                delete lhs;
                delete rhs;
            }
        }
        return result;
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
        else if(strcmp(str, "ln") == 0) {
            return new Function(Type::LN);
        }
        else if(strcmp(str, "sinh") == 0) {
            return new Function(Type::SINH);
        }
        else if(strcmp(str, "cosh") == 0) {
            return new Function(Type::COSH);
        }
        else if(strcmp(str, "tanh") == 0) {
            return new Function(Type::TANH);
        }
        else if(strcmp(str, "asinh") == 0) {
            return new Function(Type::ASINH);
        }
        else if(strcmp(str, "acosh") == 0) {
            return new Function(Type::ACOSH);
        }
        else if(strcmp(str, "atanh") == 0) {
            return new Function(Type::ATANH);
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
        case Type::LN:
        {
            return log(arg);
        }
        case Type::LOG10:
        {
            return log10(arg);
        }
        case Type::LOG2:
        {
            return log2(arg);
        }
        case Type::SINH:
        {
            return sinh(arg);
        }
        case Type::COSH:
        {
            return cosh(arg);
        }
        case Type::TANH:
        {
            return tanh(arg);
        }
        case Type::ASINH:
        {
            return asinh(arg);
        }
        case Type::ACOSH:
        {
            return acosh(arg);
        }
        case Type::ATANH:
        {
            return atanh(arg);
        }
        default: return NAN;
        }
    }

    /******************** Other Functions ********************/
    bool isDigit(char ch) {
        return (ch >= '0' && ch <= '9') || ch == '.' || ch == LCD_CHAR_EE;
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
    DynamicArray<Token*, 4>* tokensFromExpr(neda::Container *expr, const char *varname, Numerical *varval) {
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
                //Make sure to delete the DynamicArray returned after
                auto temp = tokensFromExpr((neda::Container*) exprs[index]);
                arr->merge(temp);
                delete temp;
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
                auto temp = tokensFromExpr((neda::Container*) ((neda::Fraction*) exprs[index])->getNumerator());
                arr->merge(temp);
                delete temp;
                arr->add(&RightBracket::INSTANCE);
                arr->add(&Operator::OP_DIVIDE);
                arr->add(&LeftBracket::INSTANCE);
                temp = tokensFromExpr((neda::Container*) ((neda::Fraction*) exprs[index])->getDenominator());
                arr->merge(temp);
                delete temp;
                arr->add(&RightBracket::INSTANCE);
                
                ++index;
                allowUnary = false;
                break;
            }
            case neda::ObjType::SUPERSCRIPT:
            {
                arr->add(&Operator::OP_EXPONENT);
                arr->add(&LeftBracket::INSTANCE);
                auto temp = tokensFromExpr((neda::Container*) ((neda::Superscript*) exprs[index])->getContents());
                arr->merge(temp);
                delete temp;
                arr->add(&RightBracket::INSTANCE);

                ++index;
                allowUnary = false;
                break;
            }
            case neda::ObjType::RADICAL:
            {
                neda::Radical *radical = (neda::Radical*) exprs[index];
                //Translate to a power
                //Surround contents with brackets
                arr->add(&LeftBracket::INSTANCE);
                auto temp = tokensFromExpr((neda::Container*) radical->getContents());
                arr->merge(temp);
                delete temp;
                arr->add(&RightBracket::INSTANCE);
                //Add exponent (reciprocal)
                arr->add(&Operator::OP_EXPONENT);
                arr->add(&LeftBracket::INSTANCE);
                arr->add(new Number(1));
                arr->add(&Operator::OP_DIVIDE);
                //No n - implied square root
                if(!radical->getN()) {
                    arr->add(new Number(2));
                }
                else {
                    arr->add(&LeftBracket::INSTANCE);
                    temp = tokensFromExpr((neda::Container*) radical->getN());
                    arr->merge(temp);
                    delete temp;
                    arr->add(&RightBracket::INSTANCE);
                }
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

                    //Special processing for logarithms:
                    if(strcmp(str, "log") == 0) {
                        if(end < exprs.length() && exprs[end]->getType() == neda::ObjType::SUBSCRIPT) {
                            auto sub = (neda::Container*) ((neda::Subscript*) exprs[end])->getContents();
                            auto subContents = sub->getContents();
                            //See if the base is one of the built-in ones
                            if(subContents->length() == 1 && extractChar((*subContents)[0]) == '2') {
                                arr->add(new Function(Function::Type::LOG2));
                            }
                            else if(subContents->length() == 2 && extractChar((*subContents)[0]) == '1' && extractChar((*subContents)[1]) == '0') {
                                arr->add(new Function(Function::Type::LOG10));
                            }
                            else if(subContents->length() == 1 && extractChar((*subContents)[0]) == LCD_CHAR_EULR) {
                                arr->add(new Function(Function::Type::LN));
                            }
                            //Otherwise use the log change of base property
                            //Translate to 1/log(base) * log
                            else {
                                arr->add(new Number(1));
                                //Use special high-precedence division
                                arr->add(&Operator::OP_SP_DIV);
                                //Use base 2 because why not
                                arr->add(new Function(Function::Type::LOG2));
                                arr->add(&LeftBracket::INSTANCE);
                                auto temp = tokensFromExpr(sub);
                                arr->merge(temp);
                                delete temp;
                                arr->add(&RightBracket::INSTANCE);
                                //Use special high-precedence multiplication
                                arr->add(&Operator::OP_SP_MULT);
                                arr->add(new Function(Function::Type::LOG2));
                            }
                            //Increment end so the index gets set properly afterwards
                            ++end;
                        }
                        //Default log base: 10
                        else {
                            arr->add(new Function(Function::Type::LOG10));
                        }
                        //Allow unary after functions
                        allowUnary = true;
                    }
                    //Otherwise normal processing
                    else {
                        Function *func = Function::fromString(str);
                        //Add the function if it's valid
                        if(func) {
                            arr->add(func);
                            //Allow unary operators after functions
                            allowUnary = true;
                        }
                        //Otherwise see if it's a valid constant, or if it is the additional variable
                        else {
                            //If n is nonnull it must be added, so no need for cleanup
                            Number *n = Number::constFromString(str);
                            if(n) {
                                arr->add(n);
                            }
                            else if(varval != nullptr && strcmp(str, varname) == 0) {
                                if(varval->getNumericalType() == NumericalType::NUM) {
                                    arr->add(new Number(((Number*) varval)->value));
                                }
                                else {
                                    arr->add(new Number(((Fraction*) varval)->num));
                                    //Use special division to avoid the need for brackets
                                    arr->add(&Operator::OP_SP_DIV);
                                    arr->add(new Number(((Fraction*) varval)->denom));
                                }
                            }
                            allowUnary = false;
                        }
                    }

                    delete str;

                    index = end;
                    break;
                }

                //Otherwise find the end of the number as usual
                uint16_t end = index + 1;
                for (; end < exprs.length(); ++end) {
                    //Special processing
                    if(!exprIsDigit(exprs[end])) {
                        char ch = extractChar(exprs[end]);
                        char prev = extractChar(exprs[end - 1]);
                        //Allow a plus or minus after
                        if(prev == LCD_CHAR_EE && (ch == '+' || ch == '-')) {
                            continue;
                        }
                        else {
                            break;
                        }
                    }
                }
                //+1 for null terminator
                char *numStr = new char[end - index + 1];
                for (uint16_t i = index; i < end; i++) {
                    char ch = extractChar(exprs[i]);
                    //Convert the x10^x character to a e (parsed by atof)
                    if(ch == LCD_CHAR_EE) {
                        numStr[i - index] = 'e';
                    }
                    else {
                        numStr[i - index] = ch;
                    }
                }
                //Add null terminator
                numStr[end - index] = '\0';
                //Convert to double
                double d = atof(numStr);

                delete numStr;

                arr->add(new Number(d));
                index = end;
                allowUnary = false;
                break;
            }
            default: ++index; break;
            }
        }
        return arr;
    }
}
