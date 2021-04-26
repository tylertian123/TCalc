#ifndef __NUMERICAL_H__
#define __NUMERICAL_H__

#include <stdint.h>

namespace util {

    /*
     * Represents a fraction.
     */
    struct Fraction {
        constexpr Fraction() : num(0), denom(0) {
        }
        constexpr Fraction(int64_t num, int64_t denom) : num(num), denom(denom) {
        }

        int64_t num;
        int64_t denom;

        inline operator double() const {
            return static_cast<double>(num) / denom;
        }
    };

    /*
     * A class that can represent either a fraction or a floating-point number.
     */
    class Numerical {
    public:
        /*
         * Constructs a Numerical representing the floating-point number 0.
         */
        Numerical();
        /*
         * Constucts a Numerical representing a floating point value.
         */
        Numerical(double value);
        /*
         * Constucts a Numerical representing a fraction.
         */
        Numerical(int64_t num, int64_t denom);
        /*
         * Constucts a Numerical representing a fraction.
         */
        Numerical(const Fraction &frac);

        // Copy constructor
        Numerical(const Numerical &other) = default;
        // Move constructor
        Numerical(Numerical &&other) = default;

        /*
         * Tests whether or not this Numerical represents a floating-point number.
         */
        bool isNumber() const;
        /*
         * Returns the value of this Numerical as a double.
         */
        double asDouble() const;
        /*
         * Returns the value of this Numerical as a fraction.
         *
         * Warning: This does not check whether or not a fraction is actually represented!
         */
        Fraction asFraction() const;

        /*
         * Reduces the fraction represented by this Numerical.
         * If this numerical does not represent a fraction, this method will have no effect.
         */
        void reduce();

        /*
         * Converts this Numerical to a double representation, no matter its contents.
         * If this numerical is already representing a double, this method will have no effect.
         */
        void toDouble();

        /*
         * Converts this Numerical to a fraction representation.
         * If this numerical is already representing a fraction, this method will have no effect.
         * If this numerical does not represent an integer, this method will also have no effect.
         */
        void toFraction();

        Numerical &operator=(const Numerical &other) = default;
        Numerical &operator=(double n);
        Numerical &operator=(const Fraction &frac);

        Numerical &operator+=(const Numerical &other);
        Numerical &operator+=(double n);
        Numerical &operator+=(const Fraction &frac);

        Numerical &operator-=(const Numerical &other);
        Numerical &operator-=(double n);
        Numerical &operator-=(const Fraction &frac);

        Numerical &operator*=(const Numerical &other);
        Numerical &operator*=(double n);
        Numerical &operator*=(const Fraction &frac);

        Numerical &operator/=(const Numerical &other);
        Numerical &operator/=(double n);
        Numerical &operator/=(const Fraction &frac);

        Numerical operator-() const;

        Numerical operator+(const Numerical &other) const;
        Numerical operator+(double n) const;
        Numerical operator+(const Fraction &frac) const;
        friend Numerical operator+(double n, const Numerical &num);
        friend Numerical operator+(const Fraction &frac, const Numerical &num);

        Numerical operator-(const Numerical &other) const;
        Numerical operator-(double n) const;
        Numerical operator-(const Fraction &frac) const;
        friend Numerical operator-(double n, const Numerical &num);
        friend Numerical operator-(const Fraction &frac, const Numerical &num);

        Numerical operator*(const Numerical &other) const;
        Numerical operator*(double n) const;
        Numerical operator*(const Fraction &frac) const;
        friend Numerical operator*(double n, const Numerical &num);
        friend Numerical operator*(const Fraction &frac, const Numerical &num);

        Numerical operator/(const Numerical &other) const;
        Numerical operator/(double n) const;
        Numerical operator/(const Fraction &frac) const;
        friend Numerical operator/(double n, const Numerical &num);
        friend Numerical operator/(const Fraction &frac, const Numerical &num);

        bool operator==(const Numerical &other) const;
        bool operator==(double n) const;
        bool operator==(const Fraction &frac) const;
        friend bool operator==(double n, const Numerical &num);
        friend bool operator==(const Fraction &frac, const Numerical &num);

        bool operator!=(const Numerical &other) const;
        bool operator!=(double n) const;
        bool operator!=(const Fraction &frac) const;
        friend bool operator!=(double n, const Numerical &num);
        friend bool operator!=(const Fraction &frac, const Numerical &num);

        bool operator>(const Numerical &other) const;
        bool operator>(double n) const;
        bool operator>(const Fraction &frac) const;
        friend bool operator>(double n, const Numerical &num);
        friend bool operator>(const Fraction &frac, const Numerical &num);

        bool operator<(const Numerical &other) const;
        bool operator<(double n) const;
        bool operator<(const Fraction &frac) const;
        friend bool operator<(double n, const Numerical &num);
        friend bool operator<(const Fraction &frac, const Numerical &num);

        explicit operator double() const;
        explicit operator Fraction() const;

        void sqrt();

        void pow(const Numerical &other);
        void pow(double n);
        void pow(const Fraction &frac);

        bool feq(const Numerical &other) const;
        bool feq(double n) const;
        bool feq(const Fraction &frac) const;

    protected:
        /*
         * This union represents either a double or a 64-bit signed integer.
         *
         * Each util::Numerical has two of them, the numerator (num) and
         * denominator (denom). When the Numerical is representing a fraction,
         * num stores the numerator (as an int64_t) and denom stores the
         * denominator (as an int64_t). When the Numerical is representing a
         * floating-point number, num stores the value, and denom is unused.
         *
         * Note that when the Numerical is representing a fraction, the sign
         * of the fraction is kept in the numerator. That is, the denominator
         * will always be positive (or have a sign bit of 0). Therefore, one
         * can tell what this Numerical is representing by looking at the sign
         * bit of the denominator.
         */
        union DoubleOrInt64 {
            double d;
            int64_t i;
        } num, denom;

        // This method does not check that the Numerical is indeed a fraction.
        void _reduce();

        static constexpr uint64_t IS_NUMBER_FLAG = 0x8000000000000000;
    };
} // namespace util

#endif
