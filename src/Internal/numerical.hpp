#ifndef __NUMERICAL_H__
#define __NUMERICAL_H__

#include <stdint.h>

namespace util {
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

        // Copy constructor
        Numerical(const Numerical &other);
        // Move constructor
        Numerical(Numerical &&other);
        
        /*
         * Tests whether or not this Numerical represents a floating-point number.
         */
        bool isNumber() const;
        /*
         * Returns the value of this Numerical as a double.
         */
        double asDouble() const;
        /*
         * Returns the numerator of the fraction represented.
         * If the fraction is negative, this will be negative.
         * 
         * Warning: This does not check whether or not a fraction is actually represented!
         */
        int64_t numerator() const;
        /*
         * Returns the denominator of the fraction represented.
         * Even if the fraction is negative, this value will always be positive.
         * 
         * Warning: This does not check whether or not a fraction is actually represented!
         */
        int64_t denominator() const;

        /*
         * Reduces the fraction represented by this Numerical.
         * If this numerical does not represent a fraction, this method will have no effect.
         */
        void reduce();

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
    };
}

#endif
