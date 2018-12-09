#include "eval.hpp"
#include "lcd12864_charset.hpp"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CONST_PI 3.14159265358979323846
#define CONST_E 2.71828182845904523536
#define CONST_AVOGADRO 6.022140758e23
#define CONST_ELEMCHG 1.60217662e-19
#define CONST_VLIGHT 299792458

namespace eval {

    /******************** Number ********************/
    Number* Number::constFromString(const char* str) {
        if(strcmp(str, LCD_STR_PI) == 0) {
            return new Number(CONST_PI);
        }
        else if(strcmp(str, LCD_STR_EULR) == 0) {
            return new Number(CONST_E);
        }
        else if(strcmp(str, LCD_STR_AVGO) == 0) {
            return new Number(CONST_AVOGADRO);
        }
        else if(strcmp(str, LCD_STR_ECHG) == 0) {
            return new Number(CONST_ELEMCHG);
        }
        else if(strcmp(str, LCD_STR_VLIG) == 0) {
            return new Number(CONST_VLIGHT);
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
    Token* Operator::operate(Token *lhs, Token *rhs) {
        TokenType lType = lhs->getType();
        TokenType rType = rhs->getType();
        Token *result = nullptr;
        //Two numbers: normal operation
        if(lType == TokenType::NUMBER && rType == TokenType::NUMBER) {
            //Special case for division: if the operands are whole numbers, create a fraction
            if((type == Operator::Type::DIVIDE || type == Operator::Type::SP_DIV) && isInt(((Number*) lhs)->value) && isInt(((Number*) rhs)->value)) {
                auto n = static_cast<int64_t>(((Number*) lhs)->value);
                auto d = static_cast<int64_t>(((Number*) rhs)->value);
                //See if the division yields a whole number
                //Watch out for division by zero!
                if(d == 0) {
                    if(n == 0) {
                        result = new Number(NAN);
                    }
                    else {
                        result = new Number(n > 0 ? INFINITY : -INFINITY);
                    }
                }
                else if(n % d == 0) {
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
        else if(lType == TokenType::FRACTION && rType == TokenType::FRACTION) {
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
        else if(lType == TokenType::FRACTION && rType == TokenType::NUMBER) {
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
    char extractChar(neda::NEDAObj *obj) {
        if(obj->getType() != neda::ObjType::CHAR_TYPE) {
            return '\0';
        }
        return ((neda::Character*) obj)->ch;
    }
    //Returns positive if a > b, zero if equal, and negative if a < b
    int8_t compareTokens(Token *a, Token *b) {
        double aVal = a->getType() == TokenType::NUMBER ? ((Number*) a)->value : ((Fraction*) a)->doubleVal();
        double bVal = b->getType() == TokenType::NUMBER ? ((Number*) b)->value : ((Fraction*) b)->doubleVal();
        return aVal > bVal ? 1 : bVal > aVal ? -1 : 0;
    }
    uint16_t findEquals(DynamicArray<neda::NEDAObj*> *arr) {
        uint16_t equalsIndex = 0;
        bool validName = true;
        for(auto i : *arr) {
            if(i->getType() == neda::ObjType::CHAR_TYPE && ((neda::Character*) i)->ch == '=') {
                break;
            }
            //In addition to finding the equals sign, also verify that the left hand side of the equals only contains valid
            //name characters
            if(!isNameChar(extractChar(i))) {
                validName = false;
                break;
            }
            ++equalsIndex;
        }
        //If equalsIndex is the same as the length, then the if condition was never true, return an error value
        //Or if it's at index 0 or length - 1, return null since the condition can't be complete
        //Or if the name is not valid
        if(!validName || equalsIndex == arr->length() || equalsIndex == 0 || equalsIndex == arr->length() - 1) {
            return 0xFFFF;
        }
        return equalsIndex;
    }

    Token* evaluate(neda::Container *expr, uint8_t varc, const char **varn, Token **varv) {
        return evaluate(&expr->contents, varc, varn, varv);
    }
	Token* evaluate(DynamicArray<neda::NEDAObj*> *expr, uint8_t varc, const char **varn, Token **varv) {
		DynamicArray<Token*, 4> arr;
        //Deref the result so the syntax won't be so awkward
        auto &exprs = *expr;
        uint16_t index = 0;
        //This variable keeps track of whether the last token was an operator
        //It is used for unary operators like the unary minus and plus
        bool allowUnary = true;

        while (index < exprs.length()) {
            switch (exprs[index]->getType()) {
            case neda::ObjType::L_BRACKET:
            {
                //If the last token was not an operator, then it must be an implied multiplication
                if(!allowUnary) {
                    arr.add(&Operator::OP_MULTIPLY);
                }
                
                //Look for the matching right bracket
                uint16_t nesting = 1;
                uint16_t endIndex = index + 1;
                for(; endIndex < exprs.length(); ++endIndex) {
                    if(exprs[endIndex]->getType() == neda::ObjType::L_BRACKET) {
                        ++nesting;
                    }
                    else if(exprs[endIndex]->getType() == neda::ObjType::R_BRACKET) {
                        --nesting;
                        if(!nesting) {
                            break;
                        }
                    }
                }
                //If nesting is nonzero, there must be mismatched parentheses
                if(nesting) {
                    freeTokens(&arr);
                    return nullptr;
                }
                //Create the subarray, not including the two brackets
                DynamicArray<neda::NEDAObj*> inside(exprs.begin() + index + 1, exprs.begin() + endIndex);
                Token *insideResult = evaluate(&inside, varc, varn, varv);
                if(!insideResult) {
                    freeTokens(&arr);
                    return nullptr;
                }
                arr.add(insideResult);

                index = endIndex + 1;
                //No unary after a pair
                allowUnary = false;
                break;
            }
            //Right brackets by themselves mean mismatched parentheses
            case neda::ObjType::R_BRACKET:
            {
                freeTokens(&arr);
                return nullptr;
            }
            case neda::ObjType::FRACTION:
            {
                Token *num = evaluate((neda::Container*) ((neda::Fraction*) exprs[index])->numerator, varc, varn, varv);
                Token *denom = evaluate((neda::Container*) ((neda::Fraction*) exprs[index])->denominator, varc, varn, varv);
                if(!num || !denom) {
                    if(num) {
                        delete num;
                    }
                    if(denom) {
                        delete denom;
                    }
                    freeTokens(&arr);
                    return nullptr;
                }
                arr.add(Operator::OP_DIVIDE.operate(num, denom));

                ++index;
                allowUnary = false;
                break;
            }
            case neda::ObjType::SUPERSCRIPT:
            {
                Token *exponent = evaluate((neda::Container*) ((neda::Superscript*) exprs[index])->contents, varc, varn, varv);
                if(!exponent) {
                    freeTokens(&arr);
                    return nullptr;
                }
                arr.add(&Operator::OP_EXPONENT);
                arr.add(exponent);

                ++index;
                allowUnary = false;
                break;
            }
            case neda::ObjType::RADICAL:
            {
                //If the last token was not an operator, then it must be an implied multiplication
                if(!allowUnary) {
                    arr.add(&Operator::OP_MULTIPLY);
                }
                Token *n;
                if(((neda::Radical*) exprs[index])->n) {
                    n = evaluate((neda::Container*) ((neda::Radical*) exprs[index])->n, varc, varn, varv);
                }
                //No base - implied square root
                else {
                    n = new Number(2);
                }
                Token *contents = evaluate((neda::Container*) ((neda::Radical*) exprs[index])->contents, varc, varn, varv);

                if(!n || !contents) {
                    if(n) {
                        delete n;
                    }
                    if(contents) {
                        delete contents;
                    }
                    freeTokens(&arr);
                    return nullptr;
                }
                //Take the inverse of n
                if(n->getType() == TokenType::NUMBER) {
                    ((Number*) n)->value = 1 / ((Number*) n)->value;
                }
                else {
                    int64_t temp = ((Fraction*) n)->num;
                    ((Fraction*) n)->num = ((Fraction*) n)->denom;
                    ((Fraction*) n)->denom = temp;
                }
                arr.add(Operator::OP_EXPONENT.operate(contents, n));

                ++index;
                allowUnary = false;
                break;
            }
            case neda::ObjType::CHAR_TYPE:
            {
                char ch = extractChar(exprs[index]);
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
                            arr.add(new Number(-1));
                            arr.add(&Operator::OP_MULTIPLY);
                        }
                        ++index;
                        allowUnary = false;
                        break;
                    }
                    else {
                        arr.add(op);
                        ++index;
                        allowUnary = true;
                        break;
                    }
                }

                //Find the end
                bool isNum;
                uint16_t end = findTokenEnd(&exprs, index, 1, isNum);
                char *str = new char[end - index + 1];
                for (uint16_t i = index; i < end; i++) {
                    char ch = extractChar(exprs[i]);
                    //Convert the x10^x character to a e (parsed by atof)
                    if(ch == LCD_CHAR_EE) {
                        str[i - index] = 'e';
                    }
                    else {
                        str[i - index] = ch;
                    }
                }
                //Add null terminator
                str[end - index] = '\0';

                if(!isNum) {
                    //Special processing for logarithms:
                    if(strcmp(str, "log") == 0) {
                        if(end < exprs.length() && exprs[end]->getType() == neda::ObjType::SUBSCRIPT) {
                            Token *sub = evaluate((neda::Container*) ((neda::Subscript*) exprs[end])->contents, varc, varn, varv);
                            if(!sub) {
                                freeTokens(&arr);
                                delete[] str;
                                return nullptr;
                            }
                            //Use the log change of base property
                            //Translate to 1/log(base) * log
                            arr.add(new Number(1));
                            //Use special high-precedence division
                            arr.add(&Operator::OP_SP_DIV);
                            //Use base 2 because why not
                            arr.add(new Function(Function::Type::LOG2));
                            arr.add(sub);
                            //Use special high-precedence multiplication
                            arr.add(&Operator::OP_SP_MULT);
                            arr.add(new Function(Function::Type::LOG2));
                            //Increment end so the index gets set properly afterwards
                            ++end;
                        }
                        //Default log base: 10
                        else {
                            arr.add(new Function(Function::Type::LOG10));
                        }
                        //Allow unary after functions
                        allowUnary = true;
                    }
                    //Otherwise normal processing
                    else {
                        Function *func = Function::fromString(str);
                        //Add the function if it's valid
                        if(func) {
                            arr.add(func);
                            //Allow unary operators after functions
                            allowUnary = true;
                        }
                        //Otherwise see if it's a valid constant, or if it is the additional variable
                        else {
                            //If unary operators are not allowed, which means that the previous token was not an operator,
                            //There must be an implied multiplication 
                            if(!allowUnary) {
                                arr.add(&Operator::OP_MULTIPLY);
                            }
                            //If n is nonnull it must be added, so no need for cleanup
                            Number *n = Number::constFromString(str);
                            if(n) {
                                arr.add(n);
                            }
                            else if(varc > 0) {
                                for(uint8_t i = 0; i < varc; i ++) {
                                    if(strcmp(str, varn[i]) == 0) {
                                        if(varv[i]->getType() == TokenType::NUMBER) {
                                            arr.add(new Number(((Number*) varv[i])->value));
                                        }
                                        else {
                                            arr.add(new Fraction(((Fraction*) varv[i])->num, ((Fraction*) varv[i])->denom));
                                        }
                                        break;
                                    }
                                }
                            }
                            allowUnary = false;
                        }
                    }
                }
                else {
                    arr.add(new Number(atof(str)));
                    index = end;
                    allowUnary = false;
                }
                delete[] str;
                index = end;
                break;

            }
            case neda::ObjType::SIGMA_PI:
            {
                //Evaluate the end
                Token *end = evaluate((neda::Container*) ((neda::SigmaPi*) exprs[index])->finish, varc, varn, varv);
                if(!end) {
                    freeTokens(&arr);
                    return nullptr;
                }
                //Split the starting condition at the equals sign
                auto startContents = &((neda::Container*) ((neda::SigmaPi*) exprs[index])->start)->contents;
                uint16_t equalsIndex = findEquals(startContents);
                if(equalsIndex == 0xFFFF) {
                    delete end;
                    freeTokens(&arr);
                    return nullptr;
                }
                //Attempt to evaluate the starting condition assign value
                DynamicArray<neda::NEDAObj*> startVal(startContents->begin() + equalsIndex + 1, startContents->end());
                Token *start = evaluate(&startVal, varc, varn, varv);
                if(!start) {
                    delete end;
                    freeTokens(&arr);
                    return nullptr;
                }
                //Isolate the variable name
                char *vName = new char[equalsIndex + 1];
                for(uint16_t i = 0; i < equalsIndex; i ++) {
                    vName[i] = extractChar((*startContents)[i]);
                }
                vName[equalsIndex] = '\0';

                //Construct new variable arrays
                const char **vNames = new const char*[varc + 1];
                Token **vVals = new Token*[varc + 1];
                //Copy existing
                for(uint8_t i = 0; i < varc; i ++) {
                    vNames[i] = varn[i];
                    vVals[i] = varv[i];
                }
                vNames[varc] = vName;
                vVals[varc] = start;

                auto &type = ((neda::SigmaPi*) exprs[index])->symbol;
                //Different starting values for summation and product
                Token *val = new Number(type.data == lcd::CHAR_SUMMATION.data ? 0 : 1);
                //While the start is still less than or equal to the end
                while(compareTokens(start, end) <= 0) {
                    //Evaluate the inside expression
                    Token *n = evaluate((neda::Container*) ((neda::SigmaPi*) exprs[index])->contents, varc + 1, vNames, vVals);
                    //If there is ever a syntax error then cleanup and exit
                    if(!n) {
                        delete end;
                        delete start;
                        delete[] vName;
                        delete[] vNames;
                        delete[] vVals;
                        delete val;
                        freeTokens(&arr);
                        return nullptr;
                    }
                    //Add or multiply the expressions
                    //Operate takes care of deletion
                    val = (type.data == lcd::CHAR_SUMMATION.data ? Operator::OP_PLUS : Operator::OP_MULTIPLY).operate(val, n);
                    //Add one to the start
                    if(start->getType() == TokenType::NUMBER) {
                        ++((Number*) start)->value;
                    }
                    else {
                        ((Fraction*) start)->num += ((Fraction*) start)->denom;
                    }
                }
                //Insert the value
                arr.add(val);
                //Cleanup
                delete end;
                delete start;
                delete[] vName;
                delete[] vNames;
                delete[] vVals;

                ++index;
                break;
            } 
            default: ++index; break;
            }
        }

        //After that, we should be left with an expression with nothing but Tokens, basic operators, and functions.
        //Use shunting yard
        Deque<Token*> output(arr.length());
        Deque<Token*> stack;
        for(Token *t : arr) {
            if(t->getType() == TokenType::NUMBER || t->getType() == TokenType::FRACTION) {
                output.enqueue(t);
            }
            else if(t->getType() == TokenType::FUNCTION) {
                stack.push(t);
            }
            else {
                //Operator
                while(!stack.isEmpty() && (stack.peek()->getType() == TokenType::FUNCTION 
                        || ((Operator*) stack.peek())->getPrecedence() <= ((Operator*) t)->getPrecedence())) {
                    output.enqueue(stack.pop());
                }
                stack.push(t);
            }
        }
        while(!stack.isEmpty()) {
            output.enqueue(stack.pop());
        }

        //Evaluate
        //Reuse stack
        while(!output.isEmpty()) {
            Token *t = output.dequeue();
            if(t->getType() == TokenType::NUMBER || t->getType() == TokenType::FRACTION) {
                stack.push(t);
            }
            else if(t->getType() == TokenType::FUNCTION) {
                //Syntax error: Not Enough Arguments
                if(stack.isEmpty()) {
                    //Do cleanup and return nullptr
                    delete t;
                    freeTokens(&output);
                    freeTokens(&stack);
                    return nullptr;
                }
                //Compute the function
                Token *n = stack.pop();
                Function *func = (Function*) t;
                //n Can only be a number or fraction
                if(n->getType() == TokenType::NUMBER) {
                    stack.push(new Number(func->compute(((Number*) n)->value)));
                }
                //Calling any function on a fraction converts it into a double
                else {
                    stack.push(new Number(func->compute(((Fraction*) n)->doubleVal())));
                }
                delete n;
                delete func;
            }
            else {
                if(stack.length() < 2) {
                    freeTokens(&output);
                    freeTokens(&stack);
                    return nullptr;
                }
                Token *rhs = stack.pop();
                Token *lhs = stack.pop();
                stack.push(((Operator*) t)->operate(lhs, rhs));
            }
        }

        if(stack.length() != 1) {
            //Syntax error: Too many numbers??
            while(!stack.isEmpty()) {
                delete stack.pop();
            }
            return nullptr;
        }
        return stack.pop();
	}
}
