#ifndef __EVAL_H__
#define __EVAL_H__

#include "neda.hpp"
#include "dynamarr.hpp"
#include "deque.hpp"
#include "util.hpp"
#include "lcd12864_charset.hpp"
#include <math.h>

namespace eval {

	extern bool useRadians;
    extern bool autoFractions;

	/*
	 * Base Token class and type enum
	 */
	enum TokenType : uint8_t {
		NUMBER,
		FRACTION,
		OPERATOR,
        FUNCTION,
		MATRIX,
	};
	class Token {
	public:
		virtual TokenType getType() const = 0;
	};

	class Number : public Token {
	public:
		Number(double value) : value(value) {}
        Number(const Number &other) : value(other.value) {}
		double value;

		virtual TokenType getType() const override {
			return TokenType::NUMBER;
		}
		
		static Number* constFromString(const char*);
	};
	
	class Fraction : public Token {
	public:
		Fraction(int64_t num, int64_t denom) : num(num), denom(denom) {
			reduce();
		}
        Fraction(const Fraction &other) : num(other.num), denom(other.denom) {}
		int64_t num;
		int64_t denom;

		virtual TokenType getType() const override {
			return TokenType::FRACTION;
		}

		static int64_t gcd(int64_t, int64_t);
		static int64_t lcm(int64_t, int64_t);

		double doubleVal() const;
		bool isInteger() const;
		void reduce();

		Fraction& operator+=(const Fraction&);
		Fraction& operator-=(const Fraction&);
		Fraction& operator*=(const Fraction&);
		Fraction& operator/=(const Fraction&);

		bool pow(const Fraction&);
	};

	class Operator : public Token {
	public:
		enum class Type : uint8_t {
			PLUS, MINUS, MULTIPLY, DIVIDE, EXPONENT, EQUALITY, CROSS, GT, LT, GTEQ, LTEQ, AND, OR, XOR, NOT, NEGATE, FACT,
            TRANSPOSE, INVERSE, NOT_EQUAL,
			// Special multiplication and division
			// These operators have the highest precedence
			SP_MULT, SP_DIV,
		};

		Type type;

		uint8_t getPrecedence() const;
        bool isUnary() const;

		virtual TokenType getType() const override {
			return TokenType::OPERATOR;
		}

		static const Operator* fromChar(char);

		double operate(double, double) const;
		// Returns whether the operation was successful (in the case of fractional exponentiation)
		// Ugly, I know.
		bool operateOn(Fraction*, Fraction*) const;

		// Operates on two numericals, taking into account fractions and everything
		// The returned numerical is allocated on the heap and needs to be freed
		// The input is deleted
		Token* operator()(Token*, Token*) const;
        // Operates on two numericals, taking into account fractions and everything
        // This only works when the operator is unary. For binary operators, use the other operator().
		// The returned numerical is allocated on the heap and needs to be freed
		// The input is deleted
        Token* operator()(Token*) const;
	
		constexpr Operator(Type type) : type(type) {}
	};

    constexpr Operator OP_PLUS(Operator::Type::PLUS),
                       OP_MINUS(Operator::Type::MINUS),
                       OP_MULTIPLY(Operator::Type::MULTIPLY),
                       OP_DIVIDE(Operator::Type::DIVIDE),
                       OP_EXPONENT(Operator::Type::EXPONENT),
                       OP_SP_MULT(Operator::Type::SP_MULT),
                       OP_SP_DIV(Operator::Type::SP_DIV),
                       OP_EQUALITY(Operator::Type::EQUALITY),
                       OP_CROSS(Operator::Type::CROSS),
                       OP_GT(Operator::Type::GT),
                       OP_LT(Operator::Type::LT),
                       OP_GTEQ(Operator::Type::GTEQ),
                       OP_LTEQ(Operator::Type::LTEQ),
                       OP_AND(Operator::Type::AND),
                       OP_OR(Operator::Type::OR),
                       OP_XOR(Operator::Type::XOR),
                       OP_NOT(Operator::Type::NOT),
                       OP_NEGATE(Operator::Type::NEGATE),
                       OP_FACT(Operator::Type::FACT),
                       OP_TRANSPOSE(Operator::Type::TRANSPOSE),
                       OP_INVERSE(Operator::Type::INVERSE),
                       OP_NOT_EQUAL(Operator::Type::NOT_EQUAL);

	// Even though only one instance of each type of function is needed, because there are a lot of functions, it is not worth it
	// to make it a singleton
	class Function : public Token {
	public:
		enum class Type : uint8_t {
			SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, ASINH, ACOSH, ATANH, LN, LOG10, LOG2, QUADROOT_A, QUADROOT_B,
			ROUND, MIN, MAX, FLOOR, CEIL, DET, LINSOLVE, RREF,

            // Cast this into an unit8_t for the total function count
            TOTAL_TYPE_COUNT
		};
        static constexpr uint8_t TYPE_COUNT = static_cast<uint8_t>(Type::TOTAL_TYPE_COUNT);
		// Must be in the same order as type
		static const char * const FUNCNAMES[TYPE_COUNT];
        // Used for displaying, doesn't have to contain all functions
        static const char * const FUNC_FULLNAMES[];
        // Length of FUNC_FULLNAMES
        static constexpr uint8_t TYPE_COUNT_DISPLAYABLE = 23;
		
		Function(Type type) : type(type) {}

		Type type;

		virtual TokenType getType() const override {
			return TokenType::FUNCTION;
		}

		static Function* fromString(const char*);
		uint8_t getNumArgs() const;
		Token* operator()(Token**) const;
	};

	class Matrix : public Token {
	public:
		Matrix(uint8_t m, uint8_t n) : m(m), n(n) {
			contents = new double[m * n];
			memset(contents, 0, sizeof(double) * m * n);
		}

		// Copy constructor
		Matrix(const Matrix &mat) : m(mat.m), n(mat.n) {
			contents = new double[m * n];
			memcpy(contents, mat.contents, sizeof(double) * m * n);
		}

		~Matrix() {
			delete[] contents;
		}

		const uint8_t m;
		const uint8_t n;
		
		double *contents;

		// Maps zero-based indexing to index in contents array
		inline uint16_t index_0(uint8_t x, uint8_t y) const {
			return x + y * n;
		}
		// Sets an entry
		inline void setEntry(uint8_t row, uint8_t col, double entry) {
			contents[index_0(col, row)] = entry;
		}
		inline double& getEntry(uint8_t row, uint8_t col) {
			return contents[index_0(col, row)];
		}
		inline const double& getEntry(uint8_t row, uint8_t col) const {
			return contents[index_0(col, row)];
		}
		inline double& operator[](const int index) {
			return contents[index];
		}
		inline const double& operator[](const int index) const {
			return contents[index];
		}

		static Matrix* add(const Matrix&, const Matrix&);
		static Matrix* subtract(const Matrix&, const Matrix&);
		static Matrix* multiply(const Matrix&, double);
		static Matrix* multiply(const Matrix&, const Matrix&);
		static double dot(const Matrix&, const Matrix&);
        // Note: This will modify the matrix
		double det();
		double len() const;
		static Matrix* cross(const Matrix&, const Matrix&);
		Matrix* transpose() const;
		Matrix* inv() const;

        Matrix* getRowVector(uint8_t row) const;
        Matrix* getColVector(uint8_t col) const;

		bool eliminate(bool allowSingular = true);

		virtual TokenType getType() const override {
			return TokenType::MATRIX;
		}
	
	protected:
		inline void rowSwap(uint8_t a, uint8_t b) {
			for(uint8_t i = 0; i < n; i ++) {
				swap(getEntry(a, i), getEntry(b, i));
			}
		}
		inline void rowMult(uint8_t row, double scalar) {
			for(uint8_t i = 0; i < n; i ++) {
				getEntry(row, i) *= scalar;
			}
		}
		inline void rowAdd(uint8_t a, uint8_t b, double scalar = 1) {
			for(uint8_t i = 0; i < n; i ++) {
				getEntry(a, i) += getEntry(b, i) * scalar;
			}
		}
	};

	struct UserDefinedFunction {
		UserDefinedFunction(neda::Container *expr, const char *name, uint8_t argc, const char **argn, const char *fullname)
				: expr(expr), name(name), argc(argc), argn(argn), fullname(fullname) {}
        UserDefinedFunction(const UserDefinedFunction &other) : expr(other.expr), name(other.name), argc(other.argc), argn(other.argn),
                fullname(other.fullname) {}

		neda::Container *expr;
		const char *name;
		uint8_t argc;
		const char **argn;
		const char *fullname;
	};

    struct Variable {
        Variable() : name(nullptr), value(nullptr) {}
        Variable(const char *name, Token *value) : name(name), value(value) {}
        Variable(const Variable &other) : name(other.name), value(other.value) {}

        const char *name;
        Token *value;
    };

	// This will delete the collection of tokens properly. It will destory all tokens in the array.
	void freeTokens(Deque<Token*> &q);
    // This will delete the collection of tokens properly. It will destory all tokens in the array.
	void freeTokens(DynamicArray<Token*> &q);
	bool isDigit(char);
	bool isNameChar(char);
	char extractChar(const neda::NEDAObj*);
	double extractDouble(const Token*);
	int8_t compareTokens(const Token*, const Token*);
	uint16_t findEquals(const DynamicArray<neda::NEDAObj*>&, bool forceVarName = true);
    int8_t isTruthy(const Token*);
    DynamicArray<Token*> evaluateArgs(const DynamicArray<neda::NEDAObj*>& expr, 
            uint16_t varc, const Variable *vars, uint16_t funcc, const UserDefinedFunction *funcs, uint16_t start, uint16_t &end);

	template <uint16_t Increase>
	uint16_t findTokenEnd(const DynamicArray<neda::NEDAObj*, Increase> *arr, uint16_t start, int8_t direction, bool &isNum) {
		int16_t end = start;
		for (; end < arr->length() && end >= 0; end += direction) {
			char ch = extractChar((*arr)[end]);
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
				if ((!inc && !id && !((ch == '+' || ch == '-') && extractChar((*arr)[end - direction]) == LCD_CHAR_EE))
					|| isNum && inc) {
					break;
				}
			}
		}
		return end;
	}
	
	Token* evaluate(const neda::Container*, const DynamicArray<Variable>&, const DynamicArray<UserDefinedFunction>&);
	Token* evaluate(const DynamicArray<neda::NEDAObj*>&, const DynamicArray<Variable>&, const DynamicArray<UserDefinedFunction>&);
    Token* evaluate(const neda::Container*, uint16_t, const Variable*, uint16_t, const UserDefinedFunction*);
    Token* evaluate(const DynamicArray<neda::NEDAObj*>&, uint16_t, const Variable*, uint16_t, const UserDefinedFunction*);
}

#endif
