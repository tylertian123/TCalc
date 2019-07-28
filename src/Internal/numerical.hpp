#ifndef __NUMERICAL_H__
#define __NUMERICAL_H__

#include <stdint.h>

namespace util {
    /*
     * A class that can represent either a fraction or a floating-point number.
     */
    class Numerical {
    public:
        Numerical();
        Numerical(double value);
        Numerical(int64_t num, int64_t denom);

        Numerical(const Numerical &other);
        Numerical(Numerical &&other);
        
        bool isNumber() const;

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
