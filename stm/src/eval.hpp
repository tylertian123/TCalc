#ifndef __EVAL_H__
#define __EVAL_H__

#include "deque.hpp"
#include "dynamarr.hpp"
#include "lcd12864_charset.hpp"
#include "neda.hpp"
#include "numerical.hpp"
#include "util.hpp"
#include <math.h>

namespace eval {

    extern bool useRadians;
    extern bool autoFractions;

    /*
     * Base Token class and type enum
     */
    enum TokenType : uint8_t {
        NUMERICAL,
        OPERATOR,
        FUNCTION,
        MATRIX,
    };

    class Token {
    public:
        virtual TokenType getType() const = 0;
        virtual ~Token(){};
    };

    class Numerical : public Token {
    public:
        Numerical(const util::Numerical &value) : value(value) {
        }
        Numerical(double n) : value(n) {
        }
        Numerical(const util::Fraction &f) : value(f) {
        }

        util::Numerical value;

        virtual TokenType getType() const override {
            return TokenType::NUMERICAL;
        }

        static Numerical *constFromString(const char *);
    };

    class Matrix : public Token {
    public:
        Matrix(uint8_t m, uint8_t n) : m(m), n(n) {
            contents = new util::Numerical[m * n];
        }

        // Copy constructor
        Matrix(const Matrix &mat) : m(mat.m), n(mat.n) {
            contents = new util::Numerical[m * n];
            memcpy(contents, mat.contents, sizeof(util::Numerical) * m * n);
        }

        ~Matrix() {
            delete[] contents;
        }

        const uint8_t m;
        const uint8_t n;

        util::Numerical *contents;

        // Maps zero-based indexing to index in contents array
        inline uint16_t index_0(uint8_t x, uint8_t y) const {
            return x + y * n;
        }
        // Sets an entry
        inline void setEntry(uint8_t row, uint8_t col, util::Numerical entry) {
            contents[index_0(col, row)] = entry;
        }
        inline util::Numerical &getEntry(uint8_t row, uint8_t col) {
            return contents[index_0(col, row)];
        }
        inline const util::Numerical &getEntry(uint8_t row, uint8_t col) const {
            return contents[index_0(col, row)];
        }
        inline util::Numerical &operator[](const int index) {
            return contents[index];
        }
        inline const util::Numerical &operator[](const int index) const {
            return contents[index];
        }

        static Matrix *add(const Matrix &, const Matrix &);
        static Matrix *subtract(const Matrix &, const Matrix &);
        static Matrix *multiply(const Matrix &, util::Numerical);
        static Matrix *multiply(const Matrix &, const Matrix &);
        static util::Numerical dot(const Matrix &, const Matrix &);
        static bool equality(const Matrix &, const Matrix &);
        // Note: This will modify the matrix
        util::Numerical det();
        util::Numerical len() const;
        static Matrix *cross(const Matrix &, const Matrix &);
        Matrix *transpose() const;
        Matrix *inv() const;

        Matrix *getRowVector(uint8_t row) const;
        Matrix *getColVector(uint8_t col) const;

        bool eliminate(bool allowSingular = true);

        virtual TokenType getType() const override {
            return TokenType::MATRIX;
        }

    protected:
        inline void rowSwap(uint8_t a, uint8_t b) {
            for (uint8_t i = 0; i < n; i++) {
                util::swap(getEntry(a, i), getEntry(b, i));
            }
        }
        inline void rowMult(uint8_t row, util::Numerical scalar) {
            for (uint8_t i = 0; i < n; i++) {
                getEntry(row, i) *= scalar;
            }
        }
        inline void rowAdd(uint8_t a, uint8_t b, util::Numerical scalar = 1) {
            for (uint8_t i = 0; i < n; i++) {
                getEntry(a, i) += getEntry(b, i) * scalar;
            }
        }
    };

    class Operator : public Token {
    public:
        enum class Type : uint8_t {
            PLUS,
            MINUS,
            MULTIPLY,
            DIVIDE,
            EXPONENT,
            EQUALITY,
            CROSS,
            GT,
            LT,
            GTEQ,
            LTEQ,
            AND,
            OR,
            XOR,
            NOT,
            NEGATE,
            FACT,
            TRANSPOSE,
            INVERSE,
            NOT_EQUAL,
            AUGMENT,
            // Special multiplication and division
            // These operators have the highest precedence
            SP_MULT,
            SP_DIV,
        };

        Operator(Type type) : type(type) {
        }

        Type type;

        uint8_t getPrecedence() const;
        bool isUnary() const;

        virtual TokenType getType() const override {
            return TokenType::OPERATOR;
        }

        static const Operator *fromChar(char);

        // Operates on two numericals, taking into account fractions and everything
        // The returned numerical is allocated on the heap and needs to be freed
        // The input is deleted
        Token *operator()(Token *, Token *) const;
        // Operates on two numericals, taking into account fractions and everything
        // This only works when the operator is unary. For binary operators, use the other operator().
        // The returned numerical is allocated on the heap and needs to be freed
        // The input is deleted
        Token *operator()(Token *) const;
    };

    class Function : public Token {
    public:
        enum class Type : uint8_t {
            SIN,
            COS,
            TAN,
            ASIN,
            ACOS,
            ATAN,
            ATAN2,
            SINH,
            COSH,
            TANH,
            ASINH,
            ACOSH,
            ATANH,
            LN,
            LOG10,
            LOG2,
            QUADROOTS,
            ROUND,
            MIN,
            MAX,
            FLOOR,
            CEIL,
            DET,
            LINSOLVE,
            LEASTSQUARES,
            RREF,
            MEAN,
            RAND,

            // Cast this into an unit8_t for the total function count
            TOTAL_TYPE_COUNT
        };
        static constexpr uint8_t TYPE_COUNT = static_cast<uint8_t>(Type::TOTAL_TYPE_COUNT);
        // Must be in the same order as type
        static const char *const FUNCNAMES[TYPE_COUNT];
        // Used for displaying, doesn't have to contain all functions
        static const char *const FUNC_FULLNAMES[];
        // Length of FUNC_FULLNAMES
        static constexpr uint8_t TYPE_COUNT_DISPLAYABLE = 26;

        Function(Type type) : type(type) {
        }

        Type type;

        virtual TokenType getType() const override {
            return TokenType::FUNCTION;
        }

        static Function *fromString(const char *);
        uint8_t getNumArgs() const;
        bool isVarArgs() const;

        // Evaluates the function. Assumes the input has the correct number of elements, and uses argc if the function
        // is varargs. Note: This function might modify the input.
        Token *operator()(Token **args, uint16_t argc) const;
    };

    struct UserDefinedFunction {
        UserDefinedFunction(
                neda::Container *expr, const char *name, uint8_t argc, const char **argn, const char *fullname)
                : expr(expr), name(name), argc(argc), argn(argn), fullname(fullname) {
        }
        UserDefinedFunction(const UserDefinedFunction &other)
                : expr(other.expr), name(other.name), argc(other.argc), argn(other.argn), fullname(other.fullname) {
        }

        neda::Container *expr;
        const char *name;
        uint8_t argc;
        const char **argn;
        const char *fullname;
    };

    struct Variable {
        Variable() : name(nullptr), value(nullptr) {
        }
        Variable(const char *name, Token *value) : name(name), value(value) {
        }
        Variable(const Variable &other) : name(other.name), value(other.value) {
        }

        const char *name;
        Token *value;
    };

    /*
     * Converts t into a suitable NEDA representation, which is then added to cont.
     *
     * cont - A pointer to a neda::Container to put the objects in
     * t - The Token to convert
     * significantDigits - The number of significant digits when converting floating-point values
     * forceDecimal - If set to true, all fractions will be displayed in decimal format; has precedence over
     * asMixedNumber asMixedNumber - If set to true, improper fractions will be displayed as mixed numbers
     */
    void toNEDAObjs(neda::Container *cont, Token *t, uint8_t significantDigits, bool forceDecimal = false,
            bool asMixedNumber = false);
    Token *copyToken(Token *t);
    // This will delete the collection of tokens properly. It will destory all tokens in the array.
    void freeTokens(util::Deque<Token *> &q);
    // This will delete the collection of tokens properly. It will destory all tokens in the array.
    void freeTokens(util::DynamicArray<Token *> &q);
    bool isDigit(char);
    bool isNameChar(char);
    char extractChar(const neda::NEDAObj *);
    double extractDouble(const Token *);
    uint16_t findEquals(const util::DynamicArray<neda::NEDAObj *> &, bool forceVarName = true);
    int8_t isTruthy(const Token *);
    util::DynamicArray<Token *> evaluateArgs(const util::DynamicArray<neda::NEDAObj *> &expr, uint16_t varc,
            const Variable *vars, uint16_t funcc, const UserDefinedFunction *funcs, uint16_t start, uint16_t &end,
            bool &err);

    uint16_t findTokenEnd(
            const util::DynamicArray<neda::NEDAObj *> &arr, uint16_t start, int8_t direction, bool &isNum);

    Token *evaluate(const neda::Container *, const util::DynamicArray<Variable> &,
            const util::DynamicArray<UserDefinedFunction> &);
    Token *evaluate(const util::DynamicArray<neda::NEDAObj *> &, const util::DynamicArray<Variable> &,
            const util::DynamicArray<UserDefinedFunction> &);
    Token *evaluate(const neda::Container *, uint16_t, const Variable *, uint16_t, const UserDefinedFunction *);
    Token *evaluate(const util::DynamicArray<neda::NEDAObj *> &, uint16_t, const Variable *, uint16_t,
            const UserDefinedFunction *);
} // namespace eval

#endif
