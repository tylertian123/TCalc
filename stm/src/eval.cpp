#include "eval.hpp"
#include "lcd12864_charset.hpp"
#include "ntoa.hpp"
#include "unitconv.hpp"
#include "usart.hpp"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DEG_TO_RAD(deg) ((deg) *CONST_PI / 180.0)
#define RAD_TO_DEG(rad) ((rad) *180.0 / CONST_PI)
#define TRIG_FUNC_INPUT(x) (useRadians ? (x) : DEG_TO_RAD(x))
#define TRIG_FUNC_OUTPUT(x) (useRadians ? (x) : RAD_TO_DEG(x))

namespace eval {

    bool useRadians = true;
    bool autoFractions = true;

    constexpr double CONST_PI = 3.14159265358979323846;
    constexpr double CONST_E = 2.71828182845904523536;
    constexpr double CONST_AVOGADRO = 6.022140758e23;
    constexpr double CONST_ELEMCHG = 1.60217662e-19;
    constexpr double CONST_VLIGHT = 299792458;
    constexpr double CONST_AGRAV = 9.80665;

    /******************** Numerical ********************/
    Numerical *Numerical::constFromString(const char *str) {
        if (strcmp(str, LCD_STR_PI) == 0) {
            return new Numerical(CONST_PI);
        }
        else if (strcmp(str, LCD_STR_EULR) == 0) {
            return new Numerical(CONST_E);
        }
        else if (strcmp(str, LCD_STR_AVGO) == 0) {
            return new Numerical(CONST_AVOGADRO);
        }
        else if (strcmp(str, LCD_STR_ECHG) == 0) {
            return new Numerical(CONST_ELEMCHG);
        }
        else if (strcmp(str, LCD_STR_VLIG) == 0) {
            return new Numerical(CONST_VLIGHT);
        }
        else if (strcmp(str, LCD_STR_AGV) == 0) {
            return new Numerical(CONST_AGRAV);
        }
        else {
            return nullptr;
        }
    }

    /******************** Matrix ********************/
    Matrix *Matrix::add(const Matrix &a, const Matrix &b) {
        // Make sure two matrices are the same size
        if (a.m != b.m || a.n != b.n) {
            return nullptr;
        }

        Matrix *result = new Matrix(a.m, a.n);
        for (uint16_t i = 0; i < a.m * a.n; i++) {
            (*result)[i] = a[i] + b[i];
        }
        return result;
    }
    Matrix *Matrix::subtract(const Matrix &a, const Matrix &b) {
        // Make sure two matrices are the same size
        if (a.m != b.m || a.n != b.n) {
            return nullptr;
        }

        Matrix *result = new Matrix(a.m, a.n);
        for (uint16_t i = 0; i < a.m * a.n; i++) {
            (*result)[i] = a[i] - b[i];
        }
        return result;
    }
    Matrix *Matrix::multiply(const Matrix &a, const Matrix &b) {
        // Make sure the two matrices can be multiplied with each other
        if (a.n != b.m) {
            return nullptr;
        }

        Matrix *result = new Matrix(a.m, b.n);
        for (uint8_t row = 0; row < a.m; row++) {
            for (uint8_t col = 0; col < b.n; col++) {
                // Take the dot product
                util::Numerical sum = 0;
                for (uint8_t i = 0; i < a.n; i++) {
                    sum += a.getEntry(row, i) * b.getEntry(i, col);
                }
                result->setEntry(row, col, sum);
            }
        }
        return result;
    }
    Matrix *Matrix::multiply(const Matrix &a, util::Numerical scalar) {
        Matrix *result = new Matrix(a.m, a.n);
        for (uint16_t i = 0; i < a.m * a.n; i++) {
            (*result)[i] = a[i] * scalar;
        }
        return result;
    }
    util::Numerical Matrix::dot(const Matrix &a, const Matrix &b) {
        if (a.n == 1 && b.n == 1 && a.m == b.m) {
            util::Numerical sum = 0;
            for (uint8_t i = 0; i < a.m; i++) {
                sum += a[i] * b[i];
            }
            return sum;
        }
        else {
            return NAN;
        }
    }
    bool Matrix::equality(const Matrix &a, const Matrix &b) {
        if (a.m != b.m || a.n != b.n) {
            return false;
        }
        bool equal = true;
        for (uint16_t i = 0; i < a.m * a.n; i++) {
            if (a[i].feq(b[i])) {
                equal = false;
                break;
            }
        }
        return equal;
    }
    Matrix *Matrix::leastSquares(const Matrix &a, const Matrix &b) {
        // Matrix
        Matrix *aTranspose = a.transpose();
        // Matrix
        Matrix *aTransposeA = Matrix::multiply(*aTranspose, a);
        // Vector
        Matrix *aTransposeB = Matrix::multiply(*aTranspose, b);

        delete aTranspose;
        // Matrix
        Matrix augmented(aTransposeA->m, aTransposeA->n + 1);
        for (uint8_t i = 0; i < aTransposeA->m; i++) {
            for (uint8_t j = 0; j < aTransposeA->n; j++) {
                augmented.setEntry(i, j, aTransposeA->getEntry(i, j));
            }
        }
        for (uint8_t i = 0; i < aTransposeA->m; i++) {
            augmented.setEntry(i, aTransposeA->n, (*aTransposeB)[i]);
        }
        delete aTransposeA;
        delete aTransposeB;

        if (!augmented.eliminate(false)) {
            return nullptr;
        }
        // Construct solution as vector
        Matrix *solution = new Matrix(augmented.m, 1);
        for (uint8_t i = 0; i < augmented.m; i++) {
            (*solution)[i] = augmented.getEntry(i, augmented.m);
        }
        return solution;
    }
    util::Numerical Matrix::det() {
        // No determinant for nonsquare matrices
        if (m != n) {
            return NAN;
        }

        // Apply gaussian elimination to make the matrix upper-triangular
        // Keep track of the negating
        bool neg = 0;
        // Copied from eliminate() and modified
        for (uint8_t i = 0; i < m; i++) {
            if (getEntry(i, i) == 0) {
                uint8_t k = i;
                for (; k < m; k++) {
                    if (getEntry(k, i) != 0) {
                        rowSwap(i, k);
                        // Swapping two rows negates the determinant
                        neg = !neg;
                        break;
                    }
                }

                if (k == m) {
                    // If the entire column is 0, the matrix is singular
                    // Therefore the determinant must be 0
                    return 0;
                }
            }

            util::Numerical pivot = getEntry(i, i);

            // Eliminate this column in all rows below
            // Adding to one row a scalar multiple of another does not change the determinant
            for (uint8_t k = i + 1; k < m; k++) {
                rowAdd(k, i, -(getEntry(k, i) / pivot));
            }
        }

        // Now the matrix should be upper-triangular
        // Take the product of the main diagonal to get the determinant
        util::Numerical d = 1;
        for (uint8_t i = 0; i < m; i++) {
            d *= getEntry(i, i);
        }

        return neg ? -d : d;
    }
    util::Numerical Matrix::len() const {
        if (n != 1) {
            return NAN;
        }
        util::Numerical sum = 0;
        for (uint8_t i = 0; i < m; i++) {
            sum += contents[i] * contents[i];
        }
        sum.sqrt();
        return sum;
    }
    Matrix *Matrix::cross(const Matrix &a, const Matrix &b) {
        // Only supported for 3d vectors
        if (a.m != 3 || a.n != 1 || b.m != 3 || b.n != 1) {
            return nullptr;
        }

        Matrix *result = new Matrix(3, 1);
        (*result)[0] = a[1] * b[2] - a[2] * b[1];
        (*result)[1] = a[2] * b[0] - a[0] * b[2];
        (*result)[2] = a[0] * b[1] - a[1] * b[0];
        return result;
    }
    Matrix *Matrix::transpose() const {
        Matrix *result = new Matrix(n, m);

        for (uint8_t i = 0; i < m; i++) {
            for (uint8_t j = 0; j < n; j++) {
                result->setEntry(j, i, getEntry(i, j));
            }
        }
        return result;
    }
    bool Matrix::eliminate(bool allowSingular) {
        // If there are more rows than columns, don't do anything
        if (n < m && !allowSingular) {
            return false;
        }

        for (uint8_t i = 0, j = 0; i < m && j < n; j++) {
            if (getEntry(i, j) == 0) {
                uint8_t k = i;
                for (; k < m; k++) {
                    if (getEntry(k, j) != 0) {
                        rowSwap(i, k);
                        break;
                    }
                }

                if (k == m) {
                    if (!allowSingular) {
                        return false;
                    }
                    continue;
                }
            }

            rowMult(i, 1 / getEntry(i, j));
            for (uint8_t k = 0; k < m; k++) {
                if (i == k) {
                    continue;
                }

                rowAdd(k, i, -getEntry(k, j));
            }

            i++;
        }

        return true;
    }
    Matrix *Matrix::inv() const {
        // Nonsquare matrices have no inverses
        if (m != n) {
            return nullptr;
        }
        // Create block matrix [A|I], where I is the identity matrix
        Matrix block(m, n + m);
        // Copy the original matrix
        for (uint8_t i = 0; i < m; i++) {
            for (uint8_t j = 0; j < n; j++) {
                block.setEntry(i, j, getEntry(i, j));
            }
        }
        // Copy the identity matrix
        for (uint8_t i = 0; i < m; i++) {
            block.setEntry(i, i + m, 1);
        }
        // Eliminate
        if (!block.eliminate(false)) {
            return nullptr;
        }
        // Copy inverse
        Matrix *result = new Matrix(m, n);
        for (uint8_t i = 0; i < m; i++) {
            for (uint8_t j = 0; j < n; j++) {
                result->setEntry(i, j, block.getEntry(i, j + m));
            }
        }
        return result;
    }
    Matrix *Matrix::getRowVector(uint8_t row) const {
        if (row >= m) {
            return nullptr;
        }

        Matrix *mat = new Matrix(1, n);
        for (uint8_t i = 0; i < n; i++) {
            mat->contents[i] = getEntry(row, i);
        }
        return mat;
    }
    Matrix *Matrix::getColVector(uint8_t col) const {
        if (col >= n) {
            return nullptr;
        }

        Matrix *mat = new Matrix(m, 1);
        for (uint8_t i = 0; i < m; i++) {
            mat->contents[i] = getEntry(i, col);
        }
        return mat;
    }

    /******************** Operator ********************/
    uint8_t Operator::getPrecedence() const {
        switch (type) {
        case Type::SP_MULT:
        case Type::SP_DIV:
            return 0;
        case Type::TRANSPOSE:
        case Type::INVERSE:
            return 1;
        case Type::EXPONENT:
            return 2;
        /*
         * Note:
         * Although factorial should have a higher precedence than exponentiation, ie an expression like this
         * a^b!
         * should be equivalent to
         * a^(b!)
         * and not
         * (a^b)!
         * here it's defined the opposite way.
         *
         * This is because in TCalc, an expression like this
         *  b
         * a !
         * is translated directly into
         * a^b!
         * If the factorial operator were to have higher precedence than the exponent operator, this would not make
         * sense, as the factorial clearly applies to the entire expression a^b.
         */
        case Type::FACT:
            return 3;
        case Type::NOT:
        case Type::NEGATE:
            return 4;
        case Type::AUGMENT:
            return 5;
        case Type::MULTIPLY:
        case Type::DIVIDE:
        case Type::CROSS:
            return 6;
        case Type::PLUS:
        case Type::MINUS:
            return 7;
        case Type::EQUALITY:
        case Type::NOT_EQUAL:
        case Type::LT:
        case Type::GT:
        case Type::LTEQ:
        case Type::GTEQ:
            return 8;
        case Type::AND:
            return 9;
        case Type::OR:
            return 10;
        case Type::XOR:
            return 11;

        default:
            return 0xFF;
        }
    }
    bool Operator::isUnary() const {
        switch (type) {
        case Type::NOT:
        case Type::NEGATE:
        case Type::FACT:
        case Type::TRANSPOSE:
        case Type::INVERSE:
            return true;
        default:
            return false;
        }
    }
    const Operator *Operator::fromChar(char ch) {
        switch (ch) {
        case '+':
            return new Operator(Operator::Type::PLUS);

        case '-':
            return new Operator(Operator::Type::MINUS);

        case LCD_CHAR_MUL:
        case '*':
            return new Operator(Operator::Type::MULTIPLY);

        case LCD_CHAR_DIV:
        case '/':
            return new Operator(Operator::Type::DIVIDE);

        case '^':
            return new Operator(Operator::Type::EXPONENT);

        case LCD_CHAR_CRS:
            return new Operator(Operator::Type::CROSS);

        case '>':
            return new Operator(Operator::Type::GT);

        case '<':
            return new Operator(Operator::Type::LT);

        case LCD_CHAR_GEQ:
            return new Operator(Operator::Type::GTEQ);

        case LCD_CHAR_LEQ:
            return new Operator(Operator::Type::LTEQ);

        case LCD_CHAR_LAND:
            return new Operator(Operator::Type::AND);

        case LCD_CHAR_LOR:
            return new Operator(Operator::Type::OR);

        case LCD_CHAR_LXOR:
            return new Operator(Operator::Type::XOR);

        case LCD_CHAR_LNOT:
            return new Operator(Operator::Type::NOT);

        case '!':
            return new Operator(Operator::Type::FACT);

        case '|':
            return new Operator(Operator::Type::AUGMENT);

        default:
            return nullptr;
        }
    }
    Token *Operator::operator()(Token *lhs, Token *rhs) const {
        Token *result = nullptr;
        switch (type) {
        case Type::PLUS: {
            if (lhs->getType() == TokenType::NUMERICAL && rhs->getType() == TokenType::NUMERICAL) {
                result = new Numerical(static_cast<Numerical *>(lhs)->value + static_cast<Numerical *>(rhs)->value);
            }
            else if (lhs->getType() == TokenType::MATRIX && rhs->getType() == TokenType::MATRIX) {
                result = Matrix::add(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs));
            }
            break;
        }
        case Type::MINUS: {
            if (lhs->getType() == TokenType::NUMERICAL && rhs->getType() == TokenType::NUMERICAL) {
                result = new Numerical(static_cast<Numerical *>(lhs)->value - static_cast<Numerical *>(rhs)->value);
            }
            else if (lhs->getType() == TokenType::MATRIX && rhs->getType() == TokenType::MATRIX) {
                result = Matrix::subtract(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs));
            }
            break;
        }
        // Cross product symbol is treated like regular multiplication when not on vectors
        case Type::CROSS:
        case Type::SP_MULT:
        case Type::MULTIPLY: {
            if (lhs->getType() == TokenType::NUMERICAL && rhs->getType() == TokenType::NUMERICAL) {
                result = new Numerical(static_cast<Numerical *>(lhs)->value * static_cast<Numerical *>(rhs)->value);
            }
            else if (lhs->getType() == TokenType::MATRIX && rhs->getType() == TokenType::MATRIX) {
                if (type == Type::MULTIPLY) {
                    result = Matrix::multiply(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs));
                    // If matrix multiplication is not possible, try to take the dot product
                    if (!result) {
                        auto n = Matrix::dot(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs));
                        if (!isnan(static_cast<double>(n))) {
                            result = new Numerical(n);
                        }
                    }
                }
                else {
                    result = Matrix::cross(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs));
                }
            }
            else if (lhs->getType() == TokenType::NUMERICAL && rhs->getType() == TokenType::MATRIX) {
                result = Matrix::multiply(*static_cast<Matrix *>(rhs), static_cast<Numerical *>(lhs)->value);
            }
            else {
                result = Matrix::multiply(*static_cast<Matrix *>(lhs), static_cast<Numerical *>(rhs)->value);
            }
            break;
        }
        case Type::SP_DIV:
        case Type::DIVIDE: {
            if (lhs->getType() == TokenType::NUMERICAL && rhs->getType() == TokenType::NUMERICAL) {
                // If auto fractions is off, the division of integers must not create a fraction
                // So if both operands are integers, convert them to doubles and use the builtin double / operator
                if (!autoFractions && util::isInt(static_cast<Numerical *>(lhs)->value.asDouble()) &&
                        util::isInt(static_cast<Numerical *>(rhs)->value.asDouble())) {
                    result = new Numerical(static_cast<Numerical *>(lhs)->value.asDouble() /
                                           static_cast<Numerical *>(rhs)->value.asDouble());
                }
                else {
                    result = new Numerical(static_cast<Numerical *>(lhs)->value / static_cast<Numerical *>(rhs)->value);
                }
            }
            // Only matrix divided by scalar is allowed
            else if (lhs->getType() == TokenType::MATRIX && rhs->getType() == TokenType::NUMERICAL) {
                result = Matrix::multiply(*static_cast<Matrix *>(lhs), 1 / static_cast<Numerical *>(rhs)->value);
            }
            break;
        }
        case Type::EXPONENT: {
            // Currently only scalar exponentiation is supported
            if (lhs->getType() == TokenType::NUMERICAL && rhs->getType() == TokenType::NUMERICAL) {
                static_cast<Numerical *>(lhs)->value.pow(static_cast<Numerical *>(rhs)->value);
                result = new Numerical(static_cast<Numerical *>(lhs)->value);
            }
            break;
        }
        case Type::EQUALITY: {
            // Different types is always not equal
            if (lhs->getType() != rhs->getType()) {
                result = new Numerical(false);
            }
            else if (lhs->getType() == TokenType::NUMERICAL) {
                result = new Numerical(static_cast<Numerical *>(lhs)->value.feq(static_cast<Numerical *>(rhs)->value));
            }
            else {
                result = new Numerical(Matrix::equality(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs)));
            }
            break;
        }
        case Type::NOT_EQUAL: {
            // Different types is always not equal
            if (lhs->getType() != rhs->getType()) {
                result = new Numerical(true);
            }
            else if (lhs->getType() == TokenType::NUMERICAL) {
                result = new Numerical(!static_cast<Numerical *>(lhs)->value.feq(static_cast<Numerical *>(rhs)->value));
            }
            else {
                result = new Numerical(!Matrix::equality(*static_cast<Matrix *>(lhs), *static_cast<Matrix *>(rhs)));
            }
            break;
        }
        case Type::GT: {
            // Matrices cannot be involved in any way
            if (lhs->getType() == TokenType::MATRIX || rhs->getType() == TokenType::MATRIX) {
                result = nullptr;
            }
            // If neither are matrices then both must be numbers
            else {
                result = new Numerical(static_cast<Numerical *>(lhs)->value > static_cast<Numerical *>(rhs)->value);
            }
            break;
        }
        case Type::LT: {
            // Matrices cannot be involved in any way
            if (lhs->getType() == TokenType::MATRIX || rhs->getType() == TokenType::MATRIX) {
                result = nullptr;
            }
            // If neither are matrices then both must be numbers
            else {
                result = new Numerical(static_cast<Numerical *>(lhs)->value < static_cast<Numerical *>(rhs)->value);
            }
            break;
        }
        case Type::GTEQ: {
            // Matrices cannot be involved in any way
            if (lhs->getType() == TokenType::MATRIX || rhs->getType() == TokenType::MATRIX) {
                result = nullptr;
            }
            // If neither are matrices then both must be numbers
            else {
                result = new Numerical(static_cast<Numerical *>(lhs)->value > static_cast<Numerical *>(rhs)->value ||
                                       static_cast<Numerical *>(lhs)->value.feq(static_cast<Numerical *>(rhs)->value));
            }
            break;
        }
        case Type::LTEQ: {
            // Matrices cannot be involved in any way
            if (lhs->getType() == TokenType::MATRIX || rhs->getType() == TokenType::MATRIX) {
                result = nullptr;
            }
            // If neither are matrices then both must be numbers
            else {
                result = new Numerical(static_cast<Numerical *>(lhs)->value < static_cast<Numerical *>(rhs)->value ||
                                       static_cast<Numerical *>(lhs)->value.feq(static_cast<Numerical *>(rhs)->value));
            }
            break;
        }
        case Type::AND: {
            int8_t l = isTruthy(lhs);
            int8_t r = isTruthy(rhs);

            if (l == -1 || r == -1) {
                result = new Numerical(NAN);
            }
            else {
                result = new Numerical(l && r);
            }
            break;
        }
        case Type::OR: {
            int8_t l = isTruthy(lhs);
            int8_t r = isTruthy(rhs);

            if (l == -1 || r == -1) {
                result = new Numerical(NAN);
            }
            else {
                result = new Numerical(l || r);
            }
            break;
        }
        case Type::XOR: {
            int8_t l = isTruthy(lhs);
            int8_t r = isTruthy(rhs);

            if (l == -1 || r == -1) {
                result = new Numerical(NAN);
            }
            else {
                result = new Numerical(l ^ r);
            }
            break;
        }
        case Type::AUGMENT: {
            if (lhs->getType() != TokenType::MATRIX || rhs->getType() != TokenType::MATRIX) {
                result = nullptr;
                break;
            }
            Matrix *lMat = static_cast<Matrix *>(lhs);
            Matrix *rMat = static_cast<Matrix *>(rhs);

            if (lMat->m != rMat->m) {
                result = new Numerical(NAN);
            }
            else {
                Matrix *mat = new Matrix(lMat->m, lMat->n + rMat->n);
                for (uint8_t i = 0; i < mat->m; i++) {
                    for (uint8_t j = 0; j < mat->n; j++) {
                        if (j < lMat->n) {
                            mat->setEntry(i, j, lMat->getEntry(i, j));
                        }
                        else {
                            mat->setEntry(i, j, rMat->getEntry(i, j - lMat->n));
                        }
                    }
                }
                result = mat;
            }
            break;
        }
        default:
            break;
        }

        delete lhs;
        delete rhs;
        return result;
    }
    Token *Operator::operator()(Token *t) const {
        switch (type) {
        case Type::NOT: {
            int8_t truthy = isTruthy(t);
            delete t;

            if (truthy == 1) {
                return new Numerical(false);
            }
            else if (truthy == 0) {
                return new Numerical(true);
            }
            // Undefined
            else {
                return new Numerical(NAN);
            }
        }
        case Type::NEGATE: {
            if (t->getType() == TokenType::NUMERICAL) {
                static_cast<Numerical *>(t)->value = -static_cast<Numerical *>(t)->value;
            }
            // Matrix
            else {
                for (uint16_t i = 0; i < static_cast<Matrix *>(t)->m * static_cast<Matrix *>(t)->n; i++) {
                    // Negate every entry
                    (*static_cast<Matrix *>(t))[i] = -(*static_cast<Matrix *>(t))[i];
                }
            }
            return t;
        }
        case Type::FACT: {
            double x = extractDouble(t);
            delete t;
            if (!util::isInt(x) || x < 0) {
                return new Numerical(NAN);
            }
            double d = 1;
            while (x > 0) {
                d *= x;
                --x;
            }
            return new Numerical(d);
        }
        case Type::TRANSPOSE: {
            if (t->getType() != TokenType::MATRIX) {
                return nullptr;
            }
            Matrix *mat = static_cast<Matrix *>(t);
            Matrix *result = mat->transpose();
            delete t;
            return result;
        }
        case Type::INVERSE: {
            if (t->getType() != TokenType::MATRIX) {
                return nullptr;
            }
            Matrix *mat = static_cast<Matrix *>(t);
            Matrix *result = mat->inv();
            delete t;

            return result ? static_cast<Token *>(result) : static_cast<Token *>(new Numerical(NAN));
        }
        default:
            return nullptr;
        }
    }

    /******************** Function ********************/
    // Must be in the same order as type
    const char *const Function::FUNCNAMES[TYPE_COUNT] = {"sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinh",
            "cosh", "tanh", "asinh", "acosh", "atanh", "ln",
            // log10 and log2 cannot be directly entered with a string
            "\xff", "\xff",

            "qdRts", "round", "min", "max", "floor", "ceil", "det", "linSolve", "leastSquares", "rref", "mean", "rand"};
    const char *const Function::FUNC_FULLNAMES[TYPE_COUNT_DISPLAYABLE] = {
            "sin(angle)",
            "cos(angle)",
            "tan(angle)",
            "asin(x)",
            "acos(x)",
            "atan(x)",
            "atan2(y,x)",
            "sinh(angle)",
            "cosh(angle)",
            "tanh(angle)",
            "asinh(x)",
            "acosh(x)",
            "atanh(x)",
            "ln(x)",
            "qdRts(a,b,c)",
            "round(n,decimals)",
            "min(values...)",
            "max(values...)",
            "floor(x)",
            "ceil(x)",
            "det(A)",
            "linSolve(A)",
            "leastSquares(A, b)",
            "rref(A)",
            "mean(values...)",
            "rand()",
            "linReg(x,y,model...)",
            "solve(eqn,min,max,err)"
    };
    Function *Function::fromString(const char *str) {
        for (uint8_t i = 0; i < TYPE_COUNT; i++) {
            if (strcmp(str, FUNCNAMES[i]) == 0) {
                return new Function(static_cast<Type>(i));
            }
        }
        return nullptr;
    }
    uint8_t Function::getNumArgs() const {
        switch (type) {
        case Type::QUADROOTS:
            return 3;
        case Type::ROUND:
        case Type::LEASTSQUARES:
        case Type::ATAN2:
            return 2;
        case Type::RAND:
            return 0;
        default:
            return 1;
        }
    }
    bool Function::isVarArgs() const {
        switch (type) {
        case Type::MIN:
        case Type::MAX:
        case Type::MEAN:
            return true;
        default:
            return false;
        }
    }
    Token *Function::operator()(Token **args, uint16_t argc) const {
        // Since extractDouble returns NAN if the input is a matrix, functions that don't support matrices
        // will simply return NAN
        switch (type) {
        case Type::SIN: {
            return new Numerical(sin(TRIG_FUNC_INPUT(extractDouble(args[0]))));
        }
        case Type::COS: {
            return new Numerical(cos(TRIG_FUNC_INPUT(extractDouble(args[0]))));
        }
        case Type::TAN: {
            return new Numerical(tan(TRIG_FUNC_INPUT(extractDouble(args[0]))));
        }
        case Type::ASIN: {
            return new Numerical(TRIG_FUNC_OUTPUT(asin(extractDouble(args[0]))));
        }
        case Type::ACOS: {
            return new Numerical(TRIG_FUNC_OUTPUT(acos(extractDouble(args[0]))));
        }
        case Type::ATAN: {
            return new Numerical(TRIG_FUNC_OUTPUT(atan(extractDouble(args[0]))));
        }
        case Type::ATAN2: {
            return new Numerical(TRIG_FUNC_OUTPUT(atan2(extractDouble(args[0]), extractDouble(args[1]))));
        }
        case Type::LN: {
            return new Numerical(log(extractDouble(args[0])));
        }
        case Type::LOG10: {
            return new Numerical(log10(extractDouble(args[0])));
        }
        case Type::LOG2: {
            return new Numerical(log2(extractDouble(args[0])));
        }
        case Type::SINH: {
            return new Numerical(sinh(TRIG_FUNC_INPUT(extractDouble(args[0]))));
        }
        case Type::COSH: {
            return new Numerical(cosh(TRIG_FUNC_INPUT(extractDouble(args[0]))));
        }
        case Type::TANH: {
            return new Numerical(tanh(TRIG_FUNC_INPUT(extractDouble(args[0]))));
        }
        case Type::ASINH: {
            return new Numerical(TRIG_FUNC_OUTPUT(asinh(extractDouble(args[0]))));
        }
        case Type::ACOSH: {
            return new Numerical(TRIG_FUNC_OUTPUT(acosh(extractDouble(args[0]))));
        }
        case Type::ATANH: {
            return new Numerical(TRIG_FUNC_OUTPUT(atanh(extractDouble(args[0]))));
        }
        case Type::QUADROOTS: {
            if (args[0]->getType() == TokenType::MATRIX || args[1]->getType() == TokenType::MATRIX ||
                    args[2]->getType() == TokenType::MATRIX) {
                return nullptr;
            }
            auto &a = static_cast<Numerical *>(args[0])->value, &b = static_cast<Numerical *>(args[1])->value,
                 &c = static_cast<Numerical *>(args[2])->value;
            auto disc = b * b - 4 * a * c;
            if (disc < 0) {
                return new Numerical(NAN);
            }
            disc.sqrt();
            Matrix *result = new Matrix(2, 1);
            (*result)[0] = (-b + disc) / (2 * a);
            (*result)[1] = (-b - disc) / (2 * a);
            return result;
        }
        case Type::ROUND: {
            if (args[0]->getType() == TokenType::MATRIX || args[1]->getType() == TokenType::MATRIX) {
                return nullptr;
            }
            if (!util::isInt(static_cast<Numerical *>(args[1])->value.asDouble())) {
                return new Numerical(NAN);
            }
            return new Numerical(util::round(static_cast<Numerical *>(args[0])->value.asDouble(),
                    static_cast<Numerical *>(args[1])->value.asDouble()));
        }
        case Type::DET: {
            // Syntax error: determinant of a scalar??
            if (args[0]->getType() != TokenType::MATRIX) {
                return nullptr;
            }
            Matrix *mat = static_cast<Matrix *>(args[0]);

            return new Numerical(mat->det());
        }
        case Type::LINSOLVE: {
            // Syntax error: can't solve a scalar or a matrix of the wrong dimensions
            if (args[0]->getType() != TokenType::MATRIX || ((Matrix *) args[0])->n != ((Matrix *) args[0])->m + 1) {
                return nullptr;
            }
            Matrix *mat = static_cast<Matrix *>(args[0]);
            if (!mat->eliminate(false)) {
                return new Numerical(NAN);
            }
            // Construct solution as vector
            Matrix *solution = new Matrix(mat->m, 1);
            for (uint8_t i = 0; i < mat->m; i++) {
                (*solution)[i] = mat->getEntry(i, mat->m);
            }
            return solution;
        }
        case Type::LEASTSQUARES: {
            // Matrix
            Matrix *a = static_cast<Matrix *>(args[0]);
            // Vector
            // Syntax error: can't solve a scalar or a matrix of the wrong dimensions
            Matrix *b = static_cast<Matrix *>(args[1]);
            if (args[0]->getType() != TokenType::MATRIX || args[1]->getType() != TokenType::MATRIX || b->n != 1 ||
                    a->m != b->m) {
                return nullptr;
            }

            Matrix *solution = Matrix::leastSquares(*a, *b);
            return solution ? static_cast<Token *>(solution) : static_cast<Token *>(new Numerical(NAN));
        }
        case Type::RREF: {
            // Syntax error: rref of a scalar
            if (args[0]->getType() != TokenType::MATRIX) {
                return nullptr;
            }

            // Create a copy of the matrix
            // This is because the args may be freed in the future
            Matrix *mat = new Matrix(*static_cast<Matrix *>(args[0]));
            mat->eliminate(true);
            return mat;
        }
        case Type::MIN: {
            if (args[0]->getType() != TokenType::NUMERICAL) {
                return nullptr;
            }
            util::Numerical *maxVal = &static_cast<Numerical *>(args[0])->value;
            for (uint16_t i = 1; i < argc; i++) {
                if (args[i]->getType() != TokenType::NUMERICAL) {
                    return nullptr;
                }

                if (static_cast<Numerical *>(args[i])->value < *maxVal) {
                    maxVal = &static_cast<Numerical *>(args[i])->value;
                }
            }
            return new Numerical(*maxVal);
        }
        case Type::MAX: {
            if (args[0]->getType() != TokenType::NUMERICAL) {
                return nullptr;
            }
            util::Numerical *maxVal = &static_cast<Numerical *>(args[0])->value;
            for (uint16_t i = 1; i < argc; i++) {
                if (args[i]->getType() != TokenType::NUMERICAL) {
                    return nullptr;
                }

                if (static_cast<Numerical *>(args[i])->value > *maxVal) {
                    maxVal = &static_cast<Numerical *>(args[i])->value;
                }
            }
            return new Numerical(*maxVal);
        }
        case Type::FLOOR: {
            if (args[0]->getType() == TokenType::MATRIX) {
                return nullptr;
            }
            return new Numerical(floor(extractDouble(args[0])));
        }
        case Type::CEIL: {
            if (args[0]->getType() == TokenType::MATRIX) {
                return nullptr;
            }
            return new Numerical(ceil(extractDouble(args[0])));
        }
        case Type::MEAN: {
            util::Numerical avg(0);
            for (uint16_t i = 0; i < argc; i++) {
                if (args[i]->getType() != TokenType::NUMERICAL) {
                    return nullptr;
                }
                avg += (static_cast<Numerical *>(args[i])->value - avg) / (i + 1);
            }
            return new Numerical(avg);
        }
        case Type::RAND: {
            return new Numerical(static_cast<double>(rand()) / RAND_MAX);
        }
        default:
            return new Numerical(NAN);
        }
    }

    /******************** Other Functions ********************/
    void toNEDAObjs(neda::Container *cont, Token *t, uint8_t significantDigits, bool forceDecimal, bool asMixedNumber) {
        if (!t) {
            cont->add(new neda::Character(LCD_CHAR_SERR));
            return;
        }
        if (t->getType() == TokenType::NUMERICAL) {
            const auto &num = static_cast<Numerical *>(t)->value;

            if (forceDecimal || num.isNumber() || num.asFraction().denom == 1) {
                double n = num.asDouble();

                if (isnan(n)) {
                    cont->add(new neda::Character(LCD_CHAR_MERR));
                }
                else {
                    char buf[64];
                    util::ftoa(n, buf, significantDigits, LCD_CHAR_EE);
                    cont->addString(buf);
                }
            }
            else {
                char buf[64];
                auto frac = num.asFraction();

                neda::Container *num = new neda::Container();
                neda::Container *denom = new neda::Container();

                // Display negative fractions with the minus sign in front
                if (frac.num < 0) {
                    cont->add(new neda::Character('-'));
                    frac.num = util::abs(frac.num);
                }
                // At this point the numerator should always be positive
                if (asMixedNumber && frac.num / frac.denom > 0) {
                    // Convert the improper fraction to a mixed number...
                    int64_t i = frac.num / frac.denom;
                    frac.num = frac.num % frac.denom;

                    util::dtoa(i, buf);
                    cont->addString(buf);
                }

                util::dtoa(frac.num, buf);
                num->addString(buf);
                util::dtoa(frac.denom, buf);
                denom->addString(buf);

                cont->add(new neda::Fraction(num, denom));
            }
        }
        else {
            Matrix *mat = static_cast<Matrix *>(t);
            neda::Matrix *nMat = new neda::Matrix(mat->m, mat->n);

            for (uint8_t i = 0; i < mat->m; i++) {
                for (uint8_t j = 0; j < mat->n; j++) {
                    neda::Container *c = new neda::Container();
                    Numerical n(mat->getEntry(i, j));

                    toNEDAObjs(c, &n, significantDigits, forceDecimal, asMixedNumber);

                    nMat->setEntry(i, j, c);
                }
            }
            nMat->computeDimensions();
            cont->add(nMat);
        }
    }
    Token *copyToken(Token *t) {
        if (t->getType() == TokenType::NUMERICAL) {
            return new Numerical(static_cast<Numerical *>(t)->value);
        }
        else if (t->getType() == TokenType::MATRIX) {
            return new Matrix(*static_cast<Matrix *>(t));
        }
        else if (t->getType() == TokenType::FUNCTION) {
            return new Function(static_cast<Function *>(t)->type);
        }
        else {
            // Operators
            return t;
        }
    }
    bool isDigit(char ch) {
        return (ch >= '0' && ch <= '9') || ch == '.' || ch == LCD_CHAR_EE;
    }
    // Checks if a character is a valid character for a variable/constant name.
    bool isNameChar(char ch) {
        // First check if it's a letter
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            return true;
        }
        // Check if it's a special character
        else if (ch >= LCD_CHARSET_LOWBOUND && ch <= LCD_CHARSET_HIGHBOUND) {
            // These special characters cannot be used in names
            switch (ch) {
            case LCD_CHAR_EE:
            case LCD_CHAR_MUL:
            case LCD_CHAR_DIV:
            case LCD_CHAR_CRS:
            case LCD_CHAR_LEQ:
            case LCD_CHAR_GEQ:
            case LCD_CHAR_SERR:
            case LCD_CHAR_LAND:
            case LCD_CHAR_LOR:
            case LCD_CHAR_LNOT:
            case LCD_CHAR_LXOR:
            case LCD_CHAR_RARW:
                return false;
            default:
                return true;
            }
        }
        // If neither, return false
        else {
            return false;
        }
    }
    char extractChar(const neda::NEDAObj *obj) {
        if (obj->getType() != neda::ObjType::CHAR_TYPE) {
            return '\0';
        }
        return ((neda::Character *) obj)->ch;
    }
    // Returns the double value of a Token
    // The token must be a number or fraction. Otherwise NaN will be returned.
    double extractDouble(const Token *t) {
        return t->getType() == TokenType::NUMERICAL ? static_cast<const Numerical *>(t)->value.asDouble() : NAN;
    }
    uint16_t findEquals(const util::DynamicArray<neda::NEDAObj *> &arr, bool forceVarName) {
        uint16_t equalsIndex = 0;
        bool validName = true;
        for (; equalsIndex < arr.length(); ++equalsIndex) {
            // Search for equals
            if (extractChar(arr[equalsIndex]) == '=') {
                // At the same time make sure it's not a == or a !=
                if (equalsIndex != 0 && extractChar(arr[equalsIndex - 1]) == '!') {
                    continue;
                }
                if (equalsIndex + 1 < arr.length() && extractChar(arr[equalsIndex + 1]) == '=') {
                    ++equalsIndex;
                    continue;
                }
                else {
                    break;
                }
            }
            if (forceVarName && validName) {
                // Check for name validity only if forceVarName is true
                // In addition to finding the equals sign, also verify that the left hand side of the equals only
                // contains valid name characters Allow variable names containing numbers such as a1 and a2, but the
                // number must not be the first character
                char ch = extractChar(arr[equalsIndex]);
                bool nc = isNameChar(ch);
                if ((equalsIndex == 0 && !nc) || !(nc || (ch >= '0' && ch <= '9'))) {
                    validName = false;
                    break;
                }
            }
        }
        // If equalsIndex is the same as the length, then the if condition was never true, return an error value
        // Or if it's at index 0 or length - 1, return null since the condition can't be complete
        // Or if the name is not valid
        if (!validName || equalsIndex == arr.length() || equalsIndex == 0 || equalsIndex == arr.length() - 1) {
            return 0xFFFF;
        }
        return equalsIndex;
    }
    /*
     * Tests to see if a value is "truthy".
     *
     * "Truthy" values are nonzero numbers or fractions, or any matrix/vector.
     * NaNs and infinities are undefined.
     *
     * Returns 1 if truthy, 0 if not, -1 if undefined.
     */
    int8_t isTruthy(const Token *token) {
        if (token->getType() == TokenType::MATRIX) {
            return 1;
        }

        double v = static_cast<const Numerical *>(token)->value.asDouble();

        // Infinite or NaN
        if (!isfinite(v)) {
            return -1;
        }

        return v == 0 ? 0 : 1;
    }
    // This will delete the collection of tokens properly. It will destory all tokens in the array.
    void freeTokens(util::Deque<Token *> &q) {
        while (!q.isEmpty()) {
            delete q.dequeue();
        }
    }
    // This will delete the collection of tokens properly. It will destory all tokens in the array.
    void freeTokens(util::DynamicArray<Token *> &q) {
        for (Token *t : q) {
            delete t;
        }
    }
    uint16_t findTokenEnd(
            const util::DynamicArray<neda::NEDAObj *> &arr, uint16_t start, int8_t direction, bool &isNum) {
        int16_t end = start;
        for (; end < arr.length() && end >= 0; end += direction) {
            char ch = extractChar(arr[end]);
            // Special processing for the first char
            if (end == start) {
                // The first digit has to be either a number or a name char (operators are handled)
                isNum = isDigit(ch);
            }
            else {
                // Otherwise, break if one of the two conditions:
                // The char is neither a name char or digit, and that it's not a plus or minus followed by an ee
                // Or if the token is a number and the char is a name char
                bool inc = isNameChar(ch);
                bool id = isDigit(ch);
                if ((!inc && !id && !((ch == '+' || ch == '-') && extractChar(arr[end - direction]) == LCD_CHAR_EE)) ||
                        (isNum && inc)) {
                    break;
                }
            }
        }
        return end;
    }
    /*
     * Evaluates a function arguments list, which starts with a left bracket, ends with a right bracket and is separated
     * by commas.
     *
     * expr - The NEDA expression containing the arguments list
     * varc - The number of user-defined variables
     * vars - An array of all user-defined variables
     * funcc - The number of user-defined functions
     * funcs - An array of all user-defined functions
     * start - Where to start evaluating. This should be the index of the left bracket marking the beginning of the arguments list
     * end *(out)* - A uint16_t reference which will be set to the index of the right bracket marking the end of the arguments list
     * err *(out)* - A boolean that will be set to true if an error occurs
     *
     * If there is a syntax error in the arguments list, this function will set the output bool to true.
     */
    util::DynamicArray<Token *> evaluateArgs(const util::DynamicArray<neda::NEDAObj *> &expr, const Environment &env,
            uint16_t start, uint16_t &end, bool &err) {
        
        if(start >= expr.length()) {
            err = true;
            return util::DynamicArray<Token *>();
        }
        // Args must start with a left bracket
        if (expr[start]->getType() != neda::ObjType::L_BRACKET) {
            err = true;
            return util::DynamicArray<Token *>();
        }

        uint16_t nesting = 0;
        uint16_t argStart = start + 1;
        util::DynamicArray<Token *> args;
        for(end = start; end < expr.length(); end ++) {
            // left bracket - increase nesting
            if(expr[end]->getType() == neda::ObjType::L_BRACKET) {
                ++nesting;
            }
            // right bracket - decrease nesting
            else if(expr[end]->getType() == neda::ObjType::R_BRACKET) {
                --nesting;
                // All brackets finished
                if(!nesting) {
                    // Try evaluating the last argument
                    // Special case: no arguments, right bracket right after left bracket
                    if(args.length() == 0 && end == start + 1) {
                        return args;
                    }
                    // Try evaluate
                    Token *result = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + argStart, expr.begin() + end), env);
                    if(!result) {
                        freeTokens(args);
                        err = true;
                        return util::DynamicArray<Token *>();
                    }
                    args.add(result);
                    err = false;
                    return args;
                }
            }
            // comma - handle arguments
            // Only do this if nesting level is 1, so commas inside inner brackets are not counted by mistake
            else if(expr[end]->getType() == neda::ObjType::CHAR_TYPE && extractChar(expr[end]) == ',' && nesting == 1) {
                // Try evaluate
                Token *result = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + argStart, expr.begin() + end),
                        env);
                if(!result) {
                    freeTokens(args);
                    err = true;
                    return util::DynamicArray<Token *>();
                }

                args.add(result);
                // Skip the comma
                argStart = end + 1;
            }
        }

        // Handle mismatched brackets
        err = true;
        freeTokens(args);
        return util::DynamicArray<Token *>();
    }

    Token *evaluateBuiltinFunction(const Function *func, const util::DynamicArray<neda::NEDAObj *> &expr,
            const Environment &env, uint16_t argStart, uint16_t &endOut) {
        bool err = false;
        auto args = evaluateArgs(expr, env, argStart, endOut, err);
        // Verify number of arguments
        if(err || (func->isVarArgs() ? args.length() < func->getNumArgs() : args.length() != func->getNumArgs())) {
            freeTokens(args);
            return nullptr;
        }
        // Call function
        auto result = (*func)(args.asArray(), args.length());
        freeTokens(args);
        // Will be nullptr in case of failure
        return result;
    }

    Token *evaluateUserDefinedFunction(const UserDefinedFunction *func, const util::DynamicArray<neda::NEDAObj *> &expr,
            const Environment &env, uint16_t argStart, uint16_t &endOut) {
        bool err = false;
        auto args = evaluateArgs(expr, env, argStart, endOut, err);
        // Verify number of arguments
        if(err || func->argc != args.length()) {
            freeTokens(args);
            return nullptr;
        }

        util::DynamicArray<Variable> argsArr;
        for(uint8_t i = 0; i < func->argc; i ++) {
            argsArr.add(Variable(func->argn[i], args[i]));
        }
        const Environment newEnv(env, argsArr);

        // Evaluate
        auto result = evaluate(func->expr, newEnv);
        freeTokens(args);
        // Will be nullptr in case of failure
        return result;
    }
    
    Token *logSEP(const util::DynamicArray<neda::NEDAObj *> &expr, const Environment &env, uint16_t start, uint16_t &endOut) {
        if(start + 1 < expr.length()) {
            // Custom base
            if(expr[start]->getType() == neda::ObjType::SUBSCRIPT) {
                // If subscript exists, recursively evaluate it
                Token *sub = evaluate((neda::Container *) ((neda::Subscript *) expr[start])->contents, env);
                
                // If an error occurs, clean up and return
                if (!sub) {
                    return nullptr;
                }

                // Now evaluate arguments
                bool err = false;
                auto arg = evaluateArgs(expr, env, start + 1, endOut, err);
                // Syntax error/wrong number of args
                if(err || arg.length() != 1) {
                    freeTokens(arg);
                    delete sub;
                    return nullptr;
                }
                // Change of base
                Token *n = Function(Function::Type::LOG2)(arg.asArray(), 1);
                Token *d = Function(Function::Type::LOG2)(&sub, 1);

                if(!n || !d) {
                    freeTokens(arg);
                    delete sub;
                    delete n;
                    delete d;
                }
                // Divide the result
                Token *result = Operator(Operator::Type::DIVIDE)(n, d);

                freeTokens(arg);
                delete sub;
                ++endOut;
                return result;
            }
            // Default base
            else {
                bool err = false;
                auto arg = evaluateArgs(expr, env, start, endOut, err);

                // Syntax error/wrong number of args
                if(err || arg.length() != 1) {
                    freeTokens(arg);
                    return nullptr;
                }

                Token *result = Function(Function::Type::LOG10)(arg.asArray(), 1);
                freeTokens(arg);
                ++endOut;
                return result;
            }
        }
        else {
            return nullptr;
        }
    }

    Token *linRegSEP(const util::DynamicArray<neda::NEDAObj *> &expr, const Environment &env, uint16_t start, uint16_t &endOut) {
        if(start + 1 >= expr.length() || expr[start]->getType() != neda::ObjType::L_BRACKET) {
            return nullptr;
        }

        uint16_t nesting = 0;
        uint16_t argStart = start + 1;
        util::DynamicArray<Token *> args;
        util::DynamicArray<uint32_t> model;
        for(endOut = start; endOut < expr.length(); endOut ++) {
            // left bracket - increase nesting
            if(expr[endOut]->getType() == neda::ObjType::L_BRACKET) {
                ++nesting;
            }
            // right bracket - decrease nesting
            else if(expr[endOut]->getType() == neda::ObjType::R_BRACKET) {
                --nesting;
                // All brackets finished
                if(!nesting) {
                    // Try evaluating the last argument
                    if(args.length() < 2) {
                        Token *result = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + argStart, expr.begin() + endOut),
                                env);
                        if(!result) {
                            freeTokens(args);
                            return nullptr;
                        }

                        args.add(result);
                    }
                    else {
                        // put into model
                        model.add(argStart << 16 | endOut);
                    }
                    break;
                }
            }
            // comma - handle arguments
            // Only do this if nesting level is 1, so commas inside inner brackets are not counted by mistake
            else if(expr[endOut]->getType() == neda::ObjType::CHAR_TYPE && extractChar(expr[endOut]) == ',' && nesting == 1) {
                // Try evaluate
                if(args.length() < 2) {
                    Token *result = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + argStart, expr.begin() + endOut),
                            env);
                    if(!result) {
                        freeTokens(args);
                        return nullptr;
                    }

                    args.add(result);
                }
                else {
                    // put into model
                    model.add(argStart << 16 | endOut);
                }
                // Skip the comma
                argStart = endOut + 1;
            }
        }
        ++endOut;

        // Handle errors
        if(nesting != 0 || args.length() != 2 || model.length() == 0 
                || args[0]->getType() != TokenType::MATRIX || args[1]->getType() != TokenType::MATRIX
                || static_cast<Matrix *>(args[0])->n != 1 || static_cast<Matrix *>(args[1])->n != 1
                || static_cast<Matrix *>(args[0])->m != static_cast<Matrix *>(args[1])->m
                || static_cast<Matrix *>(args[0])->m > 0xFF || model.length() > 0xFF) {
            freeTokens(args);
            return nullptr;
        }

        Matrix *x = static_cast<Matrix *>(args[0]);
        Matrix *y = static_cast<Matrix *>(args[1]);

        Matrix a(x->m, model.length());

        Numerical xn(0);
        env.args.add(Variable("x", &xn));

        for(uint8_t row = 0; row < x->m; row ++) {
            for(uint8_t col = 0; col < model.length(); col ++) {
                xn.value = x->contents[row];
                Token *t = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + (model[col] >> 16), 
                        expr.begin() + (model[col] & 0xFFFF)), env);

                if(!t || t->getType() != TokenType::NUMERICAL) {
                    freeTokens(args);
                    return nullptr;
                }

                a.setEntry(row, col, static_cast<Numerical *>(t)->value);
                delete t;
            }
        }

        env.args.removeAt(0);
        Token *result = Matrix::leastSquares(a, *y);
        freeTokens(args);
        return result ? result : new Numerical(NAN);
    }

    constexpr uint16_t BISECTION_MAX_ITERATIONS = 255;

    Token *solveSEP(const util::DynamicArray<neda::NEDAObj *> &expr, const Environment &env, uint16_t start, uint16_t &endOut) {
        if(start + 1 >= expr.length() || expr[start]->getType() != neda::ObjType::L_BRACKET) {
            return nullptr;
        }

        uint16_t nesting = 0;
        uint16_t argStart = start + 1;
        uint16_t argn = 0;
        
        uint16_t eqnEnd;
        // Since this is a numerical solver anyways keeping fractions is not a concern
        double min = 0, max = 0, err = 0;

        for(endOut = start; endOut < expr.length(); endOut ++) {
            // left bracket - increase nesting
            if(expr[endOut]->getType() == neda::ObjType::L_BRACKET) {
                ++nesting;
            }
            // right bracket - decrease nesting
            else if(expr[endOut]->getType() == neda::ObjType::R_BRACKET) {
                --nesting;
                // All brackets finished
                if(!nesting) {
                    // Try evaluating the last argument
                    if(argn > 3) {
                        // Too many arguments!
                        return nullptr;
                    }
                    
                    // Save the equation if it's the first arg
                    if(argn == 0) {
                        eqnEnd = endOut;
                    }
                    // Try evaluate
                    else {
                        Token *result = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + argStart, expr.begin() + endOut), env);
                        /// Syntax error or non-number
                        if(!result || !result->getType() == TokenType::NUMERICAL) {
                            delete result;
                            return nullptr;
                        }
                        (argn == 1 ? min : (argn == 2 ? max : err)) = static_cast<Numerical *>(result)->value.asDouble();
                        delete result;
                    }
                    argn ++;
                    break;
                }
            }
            // comma - handle arguments
            // Only do this if nesting level is 1, so commas inside inner brackets are not counted by mistake
            else if(expr[endOut]->getType() == neda::ObjType::CHAR_TYPE && extractChar(expr[endOut]) == ',' && nesting == 1) {
                if(argn > 3) {
                    // Too many arguments!
                    return nullptr;
                }
                
                // Save the equation if it's the first arg
                if(argn == 0) {
                    eqnEnd = endOut;
                }
                // Try evaluate
                else {
                    Token *result = evaluate(util::DynamicArray<neda::NEDAObj *>::createConstRef(expr.begin() + argStart, expr.begin() + endOut), env);
                    /// Syntax error or non-number
                    if(!result || !result->getType() == TokenType::NUMERICAL) {
                        delete result;
                        return nullptr;
                    }
                    (argn == 1 ? min : (argn == 2 ? max : err)) = static_cast<Numerical *>(result)->value.asDouble();
                    delete result;
                }
                argn ++;
                // Skip the comma
                argStart = endOut + 1;
            }
        }
        ++endOut;

        // Wrong bounds or wrong number of args
        // Note 3 arguments is also acceptable, in which case the accepted error is 0
        if(argn < 3 || max < min) {
            return nullptr;
        }

        // Set up equation
        const util::DynamicArray<neda::NEDAObj *> eqn = util::DynamicArray<neda::NEDAObj *>::createConstRef(
                expr.begin() + start + 1, expr.begin() + eqnEnd);
        Numerical arg(min);
        Variable varg("x", &arg);
        env.args.insert(varg, 0);

        // Evaluate on bounds of interval
        Token *t = evaluate(eqn, env);
        double minVal;
        if(!t || t->getType() != TokenType::NUMERICAL) {
            delete t;
            return nullptr;
        }
        minVal = static_cast<Numerical *>(t)->value.asDouble();
        delete t;

        arg.value = max;
        t = evaluate(eqn, env);
        double maxVal;
        if(!t || t->getType() != TokenType::NUMERICAL) {
            delete t;
            return nullptr;
        }
        maxVal = static_cast<Numerical *>(t)->value.asDouble();
        delete t;

        // Test for zeros
        if(minVal == 0) {
            env.args.removeAt(0);
            return new Numerical(minVal);
        }
        if(maxVal == 0) {
            env.args.removeAt(0);
            return new Numerical(maxVal);
        }
        // Test for same sign or infinite or NaN
        if((minVal > 0 && maxVal > 0) || (minVal < 0 && maxVal < 0) || !isfinite(minVal) || !isfinite(maxVal) || err < 0) {
            env.args.removeAt(0);
            return new Numerical(NAN);
        }

        // Start bisection
        for(uint16_t iterations = 0; iterations < BISECTION_MAX_ITERATIONS; iterations ++) {
            double x = min + (max - min) / 2;
            // Evaluate on middle
            arg.value = x;
            t = evaluate(eqn, env);
            if(!t || t->getType() != TokenType::NUMERICAL) {
                delete t;
                return nullptr;
            }
            double val = static_cast<Numerical *>(t)->value.asDouble();
            delete t;
            // Value within range
            if(util::abs(val) <= err) {
                env.args.removeAt(0);
                return new Numerical(x);
            }

            // Iterate again
            if((minVal < 0 && val > 0) || (minVal > 0 && val < 0)) {
                max = x;
                maxVal = val;
            }
            else {
                min = x;
                minVal = val;
            }
        }

        return new Numerical(min + (max - min) / 2);

        env.args.removeAt(0);
    }

    typedef Token *(*const SpecialExpressionParser)(const util::DynamicArray<neda::NEDAObj *> &expr, const Environment &env, uint16_t start, uint16_t &endOut);
    const char * const SPECIAL_EXPRESSION_NAMES[] = {
        "log",
        "linReg",
        "solve",
    };
    constexpr auto SPECIAL_EXPRESSION_LEN = sizeof(SPECIAL_EXPRESSION_NAMES) / sizeof(const char *const);
    const SpecialExpressionParser SPECIAL_EXPRESSION_PARSERS[SPECIAL_EXPRESSION_LEN] = {
        &logSEP,
        &linRegSEP,
        &solveSEP,
    };

    Token *evaluate(const neda::Container *expr, const util::DynamicArray<Variable> &vars, const util::DynamicArray<UserDefinedFunction> &funcs) {
        util::DynamicArray<eval::Variable> args;
        return evaluate(expr->contents, Environment(vars, funcs, args));
    }
    Token *evaluate(const util::DynamicArray<neda::NEDAObj *> &exprs, const util::DynamicArray<Variable> &vars, const util::DynamicArray<UserDefinedFunction> &funcs) {
        util::DynamicArray<eval::Variable> args;
        return evaluate(exprs, Environment(vars, funcs, args));
    }
    Token *evaluate(const neda::Container *expr, const Environment &env) {
        return evaluate(expr->contents, env);
    }
    // This is a label that was declared in the startup asm and exported
    // Take its address for the stack limit
    extern "C" void *__stack_limit;
    constexpr uint32_t STACK_DANGER_LIMIT = 0x00000050;
    /*
     * Evaluates an expression and returns a token result
     * Returns nullptr on syntax errors
     *
     * Parameters:
     * exprs - a reference to a util::DynamicArray of neda::NEDAObjs representing an expression
     * varc - the number of user-defined variables
     * vars - an array containing all user-defined variables
     * funcc - the number of user-defined functions
     * funcs - an array containing all user-defined functions
     */
    Token *evaluate(const util::DynamicArray<neda::NEDAObj *> &exprs, const Environment &env) {
        // This function first parses the NEDA expression to convert it into eval tokens
        // It then converts the infix notation to postfix with shunting-yard
        // And finally evaluates it and returns the result

        // First, perform a stack pointer check to make sure we don't overflow when evaluating recursive functions
        if (__current_sp() + STACK_DANGER_LIMIT <= reinterpret_cast<uint32_t>(&__stack_limit)) {
            return nullptr;
        }

        // This dynamic array holds the result of the first stage (basic parsing)
        util::DynamicArray<Token *> arr;
        uint16_t index = 0;
        // This variable keeps track of whether the last token was an operator
        bool lastTokenOperator = true;
        // Loop over every NEDA object
        // Since many types of NEDA objects require index increments of more than 1, using a while loop is more clear
        // than a for loop
        while (index < exprs.length()) {
            switch (exprs[index]->getType()) {
            // Left bracket
            case neda::ObjType::L_BRACKET: {
                // If the last token was not an operator, then it must be an implied multiplication
                if (!lastTokenOperator) {
                    // This looks very dangerous but in reality all methods of Operator are const
                    arr.add(new Operator(Operator::Type::MULTIPLY));
                }

                // Look for the matching right bracket
                uint16_t nesting = 1;
                uint16_t endIndex = index + 1;
                for (; endIndex < exprs.length(); ++endIndex) {
                    // Increase or decrease the nesting level accordingly to find the correct right bracket
                    if (exprs[endIndex]->getType() == neda::ObjType::L_BRACKET) {
                        ++nesting;
                    }
                    else if (exprs[endIndex]->getType() == neda::ObjType::R_BRACKET) {
                        --nesting;
                        // If nesting level reaches 0, then we found it
                        if (!nesting) {
                            break;
                        }
                    }
                }
                // If nesting is nonzero, there must be mismatched parentheses
                if (nesting) {
                    freeTokens(arr);
                    return nullptr;
                }
                // Construct a new array of NEDA objects that includes all object inside the brackets (but not the
                // brackets themselves!)
                const util::DynamicArray<neda::NEDAObj *> inside = util::DynamicArray<neda::NEDAObj *>::createConstRef(
                        exprs.begin() + index + 1, exprs.begin() + endIndex);
                // Recursively calculate the content inside
                Token *insideResult = evaluate(inside, env);
                // If syntax error inside bracket, clean up and return null
                if (!insideResult) {
                    freeTokens(arr);
                    return nullptr;
                }
                // Otherwise, add result to token array
                arr.add(insideResult);
                // Move on to the next part
                index = endIndex + 1;
                // Last token was not an operator
                lastTokenOperator = false;
                break;
            } // neda::ObjType::L_BRACKET

            // Right brackets
            case neda::ObjType::R_BRACKET: {
                // Since the processing for left brackets also handle their corresponding right brackets,
                // encountering a right bracket means there are mismatched parentheses.
                // Clean up and signal error.
                freeTokens(arr);
                return nullptr;
            } // neda::ObjType::R_BRACKET

            // Fractions
            case neda::ObjType::FRACTION: {
                // Recursively evaluate the numerator and denominator
                Token *num = evaluate(
                        (neda::Container *) ((neda::Fraction *) exprs[index])->numerator, env);
                Token *denom = evaluate(
                        (neda::Container *) ((neda::Fraction *) exprs[index])->denominator, env);
                // If one of them results in an error, clean up and return null
                if (!num || !denom) {
                    // Since deleting nullptrs are allowed, no need for checking
                    delete num;
                    delete denom;
                    freeTokens(arr);
                    return nullptr;
                }
                // Otherwise, call the division operator to evaluate the fraction and add it to the tokens list
                // Temporarily set autoFractions to true so a fraction is created no matter what
                bool tmp = autoFractions;
                autoFractions = true;
                arr.add(Operator(Operator::Type::DIVIDE)(num, denom));
                autoFractions = tmp;

                // Move on to the next object
                ++index;
                lastTokenOperator = false;
                break;
            } // neda::ObjType::FRACTION

            // Superscripts (exponentiation)
            case neda::ObjType::SUPERSCRIPT: {
                // If the last thing in the tokens array was a matrix, this may be a transpose or inverse operation
                if (arr.length() > 0 && arr[arr.length() - 1]->getType() == TokenType::MATRIX) {
                    const auto &c =
                            static_cast<neda::Container *>(static_cast<neda::Superscript *>(exprs[index])->contents)
                                    ->contents;

                    // Transpose
                    if (c.length() == 1 && extractChar(c[0]) == 'T') {
                        arr.add(new Operator(Operator::Type::TRANSPOSE));
                        // Break here so the rest of the code isn't executed
                        ++index;
                        lastTokenOperator = false;
                        break;
                    }
                    // Inverse
                    else if (c.length() == 2 && extractChar(c[0]) == '-' && extractChar(c[1]) == '1') {
                        arr.add(new Operator(Operator::Type::INVERSE));
                        // Break here so the rest of the code isn't executed
                        ++index;
                        lastTokenOperator = false;
                        break;
                    }
                }
                // Recursively evaluate the exponent
                Token *exponent = evaluate(
                        (neda::Container *) ((neda::Superscript *) exprs[index])->contents, env);
                // If an error occurs, clean up and return null
                if (!exponent) {
                    freeTokens(arr);
                    return nullptr;
                }
                // If the exponent is a matrix, return NaN
                // We really don't want to do the Taylor series of exp(A)
                if (exponent->getType() == TokenType::MATRIX) {
                    delete exponent;
                    freeTokens(arr);
                    return new Numerical(NAN);
                }
                // Otherwise, turn it into an exponentiation operator and the value of the exponent
                arr.add(new Operator(Operator::Type::EXPONENT));
                arr.add(exponent);
                // Move on to the next token
                ++index;
                lastTokenOperator = false;
                break;
            } // neda::ObjType::SUPERSCRIPT

            // Radicals
            case neda::ObjType::RADICAL: {
                // If the last token was not an operator there must be an implied multiplication
                if (!lastTokenOperator) {
                    arr.add(new Operator(Operator::Type::MULTIPLY));
                }
                // Used to store the base
                Token *n;
                // If the base exists, recursively evaluate it
                if (((neda::Radical *) exprs[index])->n) {
                    n = evaluate((neda::Container *) ((neda::Radical *) exprs[index])->n, env);
                }
                // No base - implied square root
                else {
                    n = new Numerical(2);
                }
                // Recursively evaluate the contents of the radical
                Token *contents = evaluate(
                        (neda::Container *) ((neda::Radical *) exprs[index])->contents, env);
                // If an error occurs, clean up and return null
                if (!n || !contents || n->getType() == TokenType::MATRIX) {
                    // nullptr deletion allowed; no need for checking
                    delete n;
                    delete contents;
                    freeTokens(arr);
                    return nullptr;
                }
                // Convert the radical into an exponentiation operation
                static_cast<Numerical *>(n)->value = 1 / static_cast<Numerical *>(n)->value;
                // Evaluate the radical and add the result to the tokens array
                arr.add(Operator(Operator::Type::EXPONENT)(contents, n));
                // Move on to the next object
                ++index;
                lastTokenOperator = false;
                break;
            } // neda::ObjType::RADICAL

            // Characters
            case neda::ObjType::CHAR_TYPE: {
                // Get the character
                char ch = extractChar(exprs[index]);
                // If the character is a space, ignore it
                if (ch == ' ') {
                    ++index;
                    break;
                }
                // See if the character is a known operator
                const Operator *op = Operator::fromChar(ch);
                // Check for equality operator which is two characters and not handled by Operator::fromChar
                if ((ch == '=' || ch == '!') && index + 1 < exprs.length() && extractChar(exprs[index + 1]) == '=') {
                    op = new Operator(ch == '=' ? Operator::Type::EQUALITY : Operator::Type::NOT_EQUAL);
                    ++index;
                }
                // Check if the character is an operator
                if (op) {
                    // Check for unary operators
                    if (lastTokenOperator && (op->type == Operator::Type::PLUS || op->type == Operator::Type::MINUS)) {
                        // Allow unary pluses, but don't do anything
                        if (op->type == Operator::Type::MINUS) {
                            arr.add(new Operator(Operator::Type::NEGATE));
                        }
                    }
                    else {
                        // Otherwise add the operator normally
                        arr.add(const_cast<Operator *>(op));
                    }
                    ++index;
                    lastTokenOperator = true;
                    break;
                }

                // Otherwise, it's probably a number or a variable
                bool isNum;
                // Find its end
                uint16_t end = findTokenEnd(exprs, index, 1, isNum);
                // Copy over the characters into a string
                char *str = new char[end - index + 1];
                for (uint16_t i = index; i < end; i++) {
                    char ch = extractChar(exprs[i]);
                    // Convert the x10^x character to a e (parsed by atof)
                    if (ch == LCD_CHAR_EE) {
                        str[i - index] = 'e';
                    }
                    else {
                        str[i - index] = ch;
                    }
                }
                // Add null terminator
                str[end - index] = '\0';

                // Special processing for identity matrix and 0 matrix notation
                if (end < exprs.length() && exprs[end]->getType() == neda::ObjType::SUBSCRIPT &&
                        (strcmp(str, "I") == 0 || strcmp(str, "0") == 0)) {
                    // Evaluate the contents of the subscript
                    Token *res = evaluate(
                            static_cast<neda::Container *>(static_cast<neda::Superscript *>(exprs[end])->contents),
                            env);
                    // Check for syntax errors, noninteger result, and out of bounds
                    double n;
                    if (!res || res->getType() == TokenType::MATRIX || (n = extractDouble(res), !util::isInt(n)) ||
                            n <= 0 || n > 255) {
                        delete res;
                        delete[] str;
                        freeTokens(arr);
                        return nullptr;
                    }
                    delete res;

                    Matrix *mat = new Matrix(static_cast<uint8_t>(n), static_cast<uint8_t>(n));
                    // If identity matrix, fill it in
                    if (str[0] == 'I') {
                        for (uint8_t i = 0; i < mat->m; i++) {
                            mat->setEntry(i, i, 1);
                        }
                    }

                    arr.add(mat);
                    delete[] str;
                    index = end + 1;
                    lastTokenOperator = false;
                    break;
                }

                // Special processing for unit conversions
                if (end < exprs.length() && extractChar(exprs[end]) == LCD_CHAR_RARW) {
                    // Find the other unit
                    index = end + 1;
                    end = findTokenEnd(exprs, index, 1, isNum);

                    // Copy the other unit
                    char *unit = new char[end - index + 1];
                    for (uint16_t i = index; i < end; i++) {
                        unit[i - index] = extractChar(exprs[i]);
                    }
                    unit[end - index] = '\0';

                    // Evaluate its arguments
                    bool err = false;
                    auto args = evaluateArgs(exprs, env, end, end, err);
                    if (err || args.length() != 1 || args[0]->getType() == TokenType::MATRIX) {
                        // Syntax error
                        freeTokens(args);
                        freeTokens(arr);
                        delete[] unit;
                        delete[] str;
                        return nullptr;
                    }

                    double result = convertUnits(extractDouble(args[0]), str, unit);
                    if (isnan(result)) {
                        // Syntax error
                        freeTokens(args);
                        freeTokens(arr);
                        delete[] unit;
                        delete[] str;
                        return nullptr;
                    }
                    // Add the result
                    arr.add(new Numerical(result));

                    freeTokens(args);
                    delete[] unit;
                    delete[] str;
                    index = end + 1;
                    lastTokenOperator = false;
                    break;
                }

                // Special expressions
                bool isSpecialExpression = false;
                for(uint16_t i = 0; i < SPECIAL_EXPRESSION_LEN; i ++) {
                    if(strcmp(str, SPECIAL_EXPRESSION_NAMES[i]) == 0) {
                        // Implied multiplication
                        if (!lastTokenOperator) {
                            arr.add(new Operator(Operator::Type::MULTIPLY));
                        }
                        // Evaluate
                        Token *result = SPECIAL_EXPRESSION_PARSERS[i](exprs, env, end, end);
                        
                        delete[] str;
                        if(!result) {
                            freeTokens(arr);
                            return nullptr;
                        }
                        arr.add(result);
                        lastTokenOperator = false;
                        index = end;
                        isSpecialExpression = true;
                        break;
                    }
                }
                if(isSpecialExpression) {
                    break;
                }

                const Function *func = nullptr;
                const UserDefinedFunction *uFunc = nullptr;
                // If the token isn't a number
                if (!isNum) {
                    func = Function::fromString(str);

                    // If it's not a normal function then try to find a user function that matches
                    if (!func) {
                        // Loop through all functions
                        for(const auto &f : env.funcs) {
                            // Compare with all the names of user-defined functions
                            if (strcmp(f.name, str) == 0) {
                                // If found, set uFunc to point to it
                                uFunc = &f;
                                break;
                            }
                        }
                    }
                    // Add the function if it's valid
                    if (func || uFunc) {
                        // Implied multiplication
                        if (!lastTokenOperator) {
                            arr.add(new Operator(Operator::Type::MULTIPLY));
                        }

                        Token *result = func ? evaluateBuiltinFunction(func, exprs, env, end, end) 
                                : evaluateUserDefinedFunction(uFunc, exprs, env, end, end);
                        
                        delete func;
                        if(!result) {
                            freeTokens(arr);
                            delete[] str;
                            return nullptr;
                        }

                        // Add result
                        arr.add(result);
                        lastTokenOperator = false;
                        // Increment end to skip the ending right bracket
                        ++end;
                    }
                    // If not a function, check if it's a constant or a variable
                    else {
                        // Implied multiplication
                        if (!lastTokenOperator) {
                            arr.add(new Operator(Operator::Type::MULTIPLY));
                        }
                        // If n is nonnull it must be added, so no need for cleanup for this dynamically allocated
                        // variable
                        Numerical *n = Numerical::constFromString(str);
                        // Add if it's a valid constant
                        if (n) {
                            arr.add(n);
                        }
                        else {
                            // Check arguments
                            for(auto &var : env.args) {
                                // Compare with each variable name
                                if (strcmp(str, var.name) == 0) {
                                    // We found a match!
                                    if (var.value->getType() == TokenType::NUMERICAL) {
                                        arr.add(new Numerical(static_cast<Numerical *>(var.value)->value));
                                    }
                                    else {
                                        arr.add(new Matrix(*static_cast<Matrix *>(var.value)));
                                    }
                                    lastTokenOperator = false;
                                    goto charParseEnd;
                                }
                            }
                            // Otherwise check if it's a valid variable
                            // Loop through all variables
                            for(auto &var : env.vars) {
                                // Compare with each variable name
                                if (strcmp(str, var.name) == 0) {
                                    // We found a match!
                                    if (var.value->getType() == TokenType::NUMERICAL) {
                                        arr.add(new Numerical(static_cast<Numerical *>(var.value)->value));
                                    }
                                    else {
                                        arr.add(new Matrix(*static_cast<Matrix *>(var.value)));
                                    }
                                    lastTokenOperator = false;
                                    goto charParseEnd;
                                }
                            }
                            // Nothing found
                            freeTokens(arr);
                            delete[] str;
                            return nullptr;
                        }
                        lastTokenOperator = false;
                    }
                }
                // If it's a number, parse it with atof and add its value
                else {
                    arr.add(new Numerical(atof(str)));
                    index = end;
                    lastTokenOperator = false;
                }
            charParseEnd:
                // Clean up the string buffer and move on
                delete[] str;
                index = end;
                break;

            } // neda::ObjType::CHAR_TYPE

            // Summation (sigma) or product (pi)
            case neda::ObjType::SIGMA_PI: {
                // First recursively evaluate the end value
                Token *end = evaluate(
                        (neda::Container *) ((neda::SigmaPi *) exprs[index])->finish, env);
                if (!end) {
                    freeTokens(arr);
                    return nullptr;
                }
                // Evaluate the starting value
                // Split the starting condition at the equals sign
                auto &startContents = ((neda::Container *) ((neda::SigmaPi *) exprs[index])->start)->contents;
                uint16_t equalsIndex = findEquals(startContents, true);
                // If equals sign not found, syntax error
                if (equalsIndex == 0xFFFF) {
                    delete end;
                    freeTokens(arr);
                    return nullptr;
                }
                // Attempt to evaluate the starting condition assign value
                const util::DynamicArray<neda::NEDAObj *> startVal =
                        util::DynamicArray<neda::NEDAObj *>::createConstRef(
                                startContents.begin() + equalsIndex + 1, startContents.end());
                Token *start = evaluate(startVal, env);
                // Check for syntax error
                if (!start) {
                    delete end;
                    freeTokens(arr);
                    return nullptr;
                }
                // Matrices are not allowed as counters
                if (start->getType() == TokenType::MATRIX || end->getType() == TokenType::MATRIX) {
                    delete end;
                    delete start;
                    freeTokens(arr);
                    return nullptr;
                }
                // Isolate the variable name
                char *vName = new char[equalsIndex + 1];
                // Extract each character
                for (uint16_t i = 0; i < equalsIndex; i++) {
                    vName[i] = extractChar(startContents[i]);
                }
                // Null termination
                vName[equalsIndex] = '\0';
                // Add it as a variable
                // Insert at index 0 so it overrides all other args
                env.args.insert(Variable(vName, start), 0);

                // Find the type of operation by extracting the symbol
                auto &type = ((neda::SigmaPi *) exprs[index])->symbol;
                // The accumulated value
                Token *val = nullptr;
                // While the start is still less than or equal to the end
                while (static_cast<Numerical *>(start)->value < static_cast<Numerical *>(end)->value ||
                        static_cast<Numerical *>(start)->value.feq(static_cast<Numerical *>(end)->value)) {
                    // Evaluate the inside expression
                    Token *n = evaluate((neda::Container *) ((neda::SigmaPi *) exprs[index])->contents, env);

                    // If there is ever a syntax error then cleanup and exit
                    if (!n) {
                        delete end;
                        delete start;
                        delete[] vName;
                        delete val;
                        freeTokens(arr);
                        return nullptr;
                    }
                    // Add or multiply the expressions if val exists
                    // Operate takes care of deletion of operands
                    if (val) {
                        val = Operator(type.data == lcd::CHAR_SUMMATION.data ? Operator::Type::PLUS
                                                                             : Operator::Type::MULTIPLY)(val, n);
                    }
                    // Set val if it doesn't exist
                    else {
                        val = n;
                    }

                    // Add one to the counter variable
                    static_cast<Numerical *>(start)->value += 1;
                }
                // Remove the counter variable added as an argument previously
                env.args.removeAt(0);
                // If val was not set, then there were no iterations
                // Set it to a default value instead
                // For summation this is 0, for product it is 1
                if (!val) {
                    val = new Numerical(type.data == lcd::CHAR_SUMMATION.data ? 0 : 1);
                }

                // Insert the value
                arr.add(val);
                // Cleanup
                delete end;
                delete start;
                delete[] vName;
                // Move on to the next object
                ++index;
                lastTokenOperator = false;
                break;
            } // neda::ObjType::SIGMA_PI

            // Matrices
            case neda::ObjType::MATRIX: {
                // Implied multiplication
                if (!lastTokenOperator) {
                    arr.add(new Operator(Operator::Type::MULTIPLY));
                }
                neda::Matrix *nMat = static_cast<neda::Matrix *>(exprs[index]);
                // Convert to a eval::Matrix
                Matrix *mat = new Matrix(nMat->m, nMat->n);
                // Matrices can be constructed from a bunch of column vectors too
                bool fromVecs = false;
                // Evaluate every entry
                for (uint16_t i = 0; i < nMat->m * nMat->n; i++) {
                    Token *n = evaluate((neda::Container *) nMat->contents[i], env);
                    // Check for syntax error
                    if (!n) {
                        delete mat;
                        freeTokens(arr);
                        return nullptr;
                    }
                    if (!fromVecs) {
                        // Matrices can't be inside matrices...
                        if (n->getType() == TokenType::MATRIX) {
                            // Unless the matrix inside is actually a column vector
                            // And we're on the first entry
                            // And the neda::Matrix only has one row
                            // In which case the matrix would be constructed using column vectors
                            if (i == 0 && static_cast<Matrix *>(n)->n == 1 && nMat->m == 1) {
                                fromVecs = true;
                                // Reconstruct the eval::Matrix
                                delete mat;
                                mat = new Matrix(static_cast<Matrix *>(n)->m, nMat->n);
                                goto constructMatrixFromVectors;
                            }
                            delete mat;
                            delete n;
                            freeTokens(arr);
                            return nullptr;
                        }
                        mat->contents[i] = static_cast<Numerical *>(n)->value;
                    }
                    else {
                        // Check that the vector only has 1 column and the rows are as expected
                        if (n->getType() != TokenType::MATRIX || static_cast<Matrix *>(n)->n != 1 ||
                                static_cast<Matrix *>(n)->m != mat->m) {
                            delete mat;
                            delete n;
                            freeTokens(arr);
                            return nullptr;
                        }
                    constructMatrixFromVectors:
                        // Fill in the column of the matrix with the entires in this column vector
                        for (uint8_t row = 0; row < mat->m; row++) {
                            mat->setEntry(row, i, static_cast<Matrix *>(n)->contents[row]);
                        }
                    }
                    delete n;
                }
                // Insert value
                arr.add(mat);
                // Move on to the next object
                ++index;
                lastTokenOperator = false;
                break;
            } // neda::ObjType::MATRIX

            // Piecewise functions
            case neda::ObjType::PIECEWISE: {
                // Implied multiplication
                if (!lastTokenOperator) {
                    arr.add(new Operator(Operator::Type::MULTIPLY));
                }
                neda::Piecewise *p = static_cast<neda::Piecewise *>(exprs[index]);

                Token *val = nullptr;
                for (uint8_t i = 0; i < p->pieces; i++) {

                    // Evaluate the condition
                    Token *n = evaluate(static_cast<neda::Container *>(p->conditions[i]), env);
                    bool isElse = false;
                    // Syntax error
                    if (!n) {
                        // See if the condition is just "else"
                        // Of course, this doesn't work if this is the first piece
                        neda::Container *condition = static_cast<neda::Container *>(p->conditions[i]);
                        if (i != 0 && condition->contents.length() == 4 && extractChar(condition->contents[0]) == 'e' &&
                                extractChar(condition->contents[1]) == 'l' &&
                                extractChar(condition->contents[2]) == 's' &&
                                extractChar(condition->contents[3]) == 'e') {
                            isElse = true;
                        }
                        else {
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                    // If it's an else clause condition is directly set to true
                    int8_t condition = isElse ? 1 : isTruthy(n);
                    delete n;
                    // Condition undefined
                    // Then the entire expression is undefined
                    if (condition == -1) {
                        freeTokens(arr);
                        return new Numerical(NAN);
                    }
                    // Condition is true
                    else if (condition == 1) {
                        // Evaluate value
                        val = evaluate(static_cast<neda::Container *>(p->values[i]), env);

                        // Syntax error
                        if (!val) {
                            freeTokens(arr);
                            return nullptr;
                        }
                        break;
                    }
                    // If condition is false, move on
                }
                // No condition was true - value is undefined
                if (!val) {
                    freeTokens(arr);
                    return new Numerical(NAN);
                }

                arr.add(val);
                ++index;
                lastTokenOperator = false;
                break;
            } // neda::ObjType::PIECEWISE

            // Subscripts
            case neda::ObjType::SUBSCRIPT: {
                // Since if the subscript was part of a log expression, it would already be handled by
                // neda::ObjType::CHAR_TYPE, currently the only use for the subscript is for matrix indices Check the
                // last item in the array and make sure it's a matrix
                if (arr[arr.length() - 1]->getType() != TokenType::MATRIX) {
                    freeTokens(arr);
                    return nullptr;
                }
                auto &contents = static_cast<neda::Container *>(static_cast<neda::Subscript *>(exprs[index])->contents)
                                         ->contents;
                // See if there's a comma in the expression
                uint16_t commaIndex;
                // Find the comma
                // Take care of brackets
                uint16_t nesting = 0;
                for (commaIndex = 0; commaIndex < contents.length(); commaIndex++) {

                    if (contents[commaIndex]->getType() == neda::ObjType::L_BRACKET) {
                        nesting++;
                    }
                    else if (contents[commaIndex]->getType() == neda::ObjType::R_BRACKET) {
                        if (nesting > 0) {
                            nesting--;
                        }
                        else {
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                    else if (nesting == 0 && extractChar(contents[commaIndex]) == ',') {
                        break;
                    }
                }

                // Get the matrix
                const Matrix *mat = static_cast<const Matrix *>(arr[arr.length() - 1]);

                Token *result = nullptr;

                // No comma
                if (commaIndex == contents.length()) {
                    Token *t = evaluate(contents, env);
                    // Check for syntax errors in expression, or noninteger result
                    if (!t || t->getType() == TokenType::MATRIX || !util::isInt(extractDouble(t))) {
                        delete t;
                        freeTokens(arr);
                        return nullptr;
                    }

                    double d = extractDouble(t);
                    if (!util::canCastProperly<double, uint8_t>(d - 1)) {
                        freeTokens(arr);
                        delete t;
                        return nullptr;
                    }
                    uint8_t index = static_cast<uint8_t>(d - 1);
                    delete t;

                    // For vectors, just take the number
                    if (mat->n == 1) {
                        if (index < mat->m) {
                            result = new Numerical((*mat)[index]);
                        }
                        else {
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                    else {
                        // Otherwise take a row vector
                        result = mat->getRowVector(index);
                        if (!result) {
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                }
                else {
                    const util::DynamicArray<neda::NEDAObj *> rowExpr =
                            util::DynamicArray<neda::NEDAObj *>::createConstRef(
                                    contents.begin(), contents.begin() + commaIndex);
                    const util::DynamicArray<neda::NEDAObj *> colExpr =
                            util::DynamicArray<neda::NEDAObj *>::createConstRef(
                                    contents.begin() + commaIndex + 1, contents.end());

                    Token *row = evaluate(rowExpr, env);
                    // Check for syntax errors in expression, or noninteger result
                    if (!row || row->getType() == TokenType::MATRIX || !util::isInt(extractDouble(row))) {
                        // Wildcard syntax
                        if (rowExpr.length() == 1 && extractChar(rowExpr[0]) == '*') {
                            row = nullptr;
                        }
                        else {
                            delete row;
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                    Token *col = evaluate(colExpr, env);
                    if (!col || col->getType() == TokenType::MATRIX || !util::isInt(extractDouble(col))) {
                        // Wildcard syntax
                        if (colExpr.length() == 1 && extractChar(colExpr[0]) == '*') {
                            col = nullptr;
                        }
                        else {
                            delete row;
                            delete col;
                            freeTokens(arr);
                            return nullptr;
                        }
                    }

                    // Wildcard on column
                    // Take row vector
                    if (row && !col) {
                        // Verify cast into uint8_t
                        double drow = extractDouble(row);
                        if (!util::canCastProperly<double, uint8_t>(drow - 1)) {
                            delete row;
                            freeTokens(arr);
                            return nullptr;
                        }

                        uint8_t rowInt = static_cast<uint8_t>(drow - 1);
                        result = mat->getRowVector(rowInt);
                        // If out of range, syntax error
                        if (!result) {
                            delete row;
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                    // Wildcard on row
                    // Take column vector
                    else if (col && !row) {
                        double dcol = extractDouble(col);
                        if (!util::canCastProperly<double, uint8_t>(dcol - 1)) {
                            delete col;
                            freeTokens(arr);
                            return nullptr;
                        }

                        uint8_t colInt = static_cast<uint8_t>(dcol - 1);
                        result = mat->getColVector(colInt);
                        if (!result) {
                            delete col;
                            freeTokens(arr);
                            return nullptr;
                        }
                    }
                    // Wildcard on both indices
                    // Just copy the matrix itself
                    else if (!col && !row) {
                        result = new Matrix(*mat);
                    }
                    else {
                        double drow = extractDouble(row);
                        double dcol = extractDouble(col);
                        // Verify that the indices can be properly casted into uint8_ts
                        if (!util::canCastProperly<double, uint8_t>(drow - 1) ||
                                !util::canCastProperly<double, uint8_t>(dcol - 1)) {
                            delete row;
                            delete col;
                            freeTokens(arr);
                            return nullptr;
                        }

                        uint8_t rowInt = static_cast<uint8_t>(drow - 1);
                        uint8_t colInt = static_cast<uint8_t>(dcol - 1);
                        if (rowInt >= mat->m || colInt >= mat->n) {
                            delete row;
                            delete col;
                            freeTokens(arr);
                            return nullptr;
                        }
                        else {
                            result = new Numerical(mat->getEntry(rowInt, colInt));
                        }
                    }
                    delete row;
                    delete col;
                }

                // Delete the matrix
                delete mat;
                // Replace it with the result
                arr[arr.length() - 1] = result;

                lastTokenOperator = false;
                index++;
                break;
            } // neda::ObjType::SUBSCRIPT

            // Absolute value
            case neda::ObjType::ABS: {
                // Implied multiplication
                if (!lastTokenOperator) {
                    arr.add(new Operator(Operator::Type::MULTIPLY));
                }
                Token *t = evaluate(static_cast<neda::Container *>(static_cast<neda::Abs *>(exprs[index])->contents), env);

                if (!t) {
                    freeTokens(arr);
                    return nullptr;
                }

                // Take the absolute value
                if (t->getType() == TokenType::NUMERICAL) {
                    auto &num = static_cast<Numerical *>(t)->value;
                    if (num.isNumber()) {
                        num = util::abs(num.asDouble());
                    }
                    else {
                        auto frac = num.asFraction();
                        frac.num = util::abs(frac.num);
                        num = frac;
                    }
                    arr.add(t);
                }
                else {
                    arr.add(new Numerical(static_cast<Matrix *>(t)->len()));
                    delete t;
                }

                lastTokenOperator = false;
                index++;
                break;
            } // neda::ObjType::ABS

            default:
                ++index;
                break;
            }
        }

        // After that, we should be left with an expression with nothing but numbers, fractions and basic operators
        // Use shunting yard
        util::Deque<Token *> output(arr.length());
        util::Deque<Token *> stack;
        bool expectOperand = true;
        for (Token *t : arr) {
            // If token is a number, fraction or matrix, put it in the queue
            if (t->getType() == TokenType::NUMERICAL || t->getType() == TokenType::MATRIX) {
                if (!expectOperand) {
                    // Syntax error
                    freeTokens(arr);
                    return nullptr;
                }
                output.enqueue(t);
                expectOperand = false;
            }
            else {
                if (static_cast<const Operator *>(t)->isUnary()) {
                    if (!expectOperand) {
                        // Syntax error
                        freeTokens(arr);
                        return nullptr;
                    }
                    stack.push(t);
                }
                else {
                    if (expectOperand) {
                        // Syntax error
                        freeTokens(arr);
                        return nullptr;
                    }
                    // Operator
                    // Pop all items on the stack that have higher precedence and put into the output queue
                    while (!stack.isEmpty() && static_cast<const Operator *>(stack.peek())->getPrecedence() <=
                            static_cast<const Operator *>(t)->getPrecedence()) {
                        output.enqueue(stack.pop());
                    }
                    // Push the operator
                    stack.push(t);
                    expectOperand = true;
                }
            }
        }
        // Transfer all the contents of the stack to the queue
        while (!stack.isEmpty()) {
            output.enqueue(stack.pop());
        }

        // Evaluate
        // Reuse stack
        while (!output.isEmpty()) {
            // Read a token
            Token *t = output.dequeue();
            // If token is a number, fraction or matrix, push onto stack
            if (t->getType() == NUMERICAL || t->getType() == TokenType::MATRIX) {
                stack.push(t);
            }
            // Operator
            else {
                // Unary operator
                if (static_cast<Operator *>(t)->isUnary()) {
                    // If there aren't enough operators, syntax error
                    if (stack.length() < 1) {
                        delete t;
                        freeTokens(output);
                        freeTokens(stack);
                        return nullptr;
                    }
                    // Pop the operand
                    Token *operand = stack.pop();
                    // Operate and push
                    Token *result = (*static_cast<Operator *>(t))(operand);
                    // Syntax error
                    if (result) {
                        stack.push(result);
                    }
                    else {
                        delete t;
                        freeTokens(output);
                        freeTokens(stack);
                        return nullptr;
                    }
                }
                else {
                    // If there aren't enough operands, syntax error
                    if (stack.length() < 2) {
                        delete t;
                        freeTokens(output);
                        freeTokens(stack);
                        return nullptr;
                    }
                    // Pop the left and right hand side operands
                    Token *rhs = stack.pop();
                    Token *lhs = stack.pop();
                    // Operate and push
                    Token *result = (*static_cast<Operator *>(t))(lhs, rhs);
                    // Check syntax error
                    if (result) {
                        stack.push(result);
                    }
                    else {
                        delete t;
                        freeTokens(output);
                        freeTokens(stack);
                        return nullptr;
                    }
                }
                delete t;
            }
        }

        if (stack.length() != 1) {
            // Syntax error: Too many numbers??
            while (!stack.isEmpty()) {
                delete stack.pop();
            }
            return nullptr;
        }
        return stack.pop();
    }
} // namespace eval
