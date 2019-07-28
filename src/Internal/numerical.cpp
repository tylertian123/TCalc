#include "numerical.hpp"
#include "util.hpp"

namespace util {

    Numerical::Numerical() {
        num.d = 0;
        // Mark as number
        denom.i = IS_NUMBER_FLAG;
    }
    Numerical::Numerical(double val) {
        num.d = val;
        // Mark as number
        denom.i = IS_NUMBER_FLAG;
    }
    Numerical::Numerical(int64_t num, int64_t denom) {
        this->num.i = num;
        this->denom.i = denom;
        // Reduce the fraction without checking for validity
        _reduce();
    }
    Numerical::Numerical(const Fraction &frac) : Numerical(frac.num, frac.denom) {}

    bool Numerical::isNumber() const {
        // See docs for Numerical::DoubleOrInt64
        return denom.i & IS_NUMBER_FLAG;
    }

    double Numerical::asDouble() const {
        if(isNumber()) {
            return num.d;
        }
        else {
            return static_cast<double>(num.i) / denom.i;
        }
    }

    Fraction Numerical::asFraction() const {
        return { num.i, denom.i };
    }

    void Numerical::reduce() {
        if(isNumber()) {
            _reduce();
        }
    }
    
    void Numerical::_reduce() {
        // Make sure the denominator is always positive
		if(denom.i < 0) {
			num.i *= -1;
			denom.i *= -1;
		}

		// Now that the denominator is positive, we can make sure the result we get is also positive
		int64_t divisor = util::abs(util::gcd(num.i, denom.i));
		if(divisor == 1) {
			return;
		}
		num.i /= divisor;
		denom.i /= divisor;
    }

    Numerical& Numerical::operator=(double n) {
        num.d = n;
        // Mark as number
        denom.i = IS_NUMBER_FLAG;
        return *this;
    }
    
    Numerical& Numerical::operator=(const Fraction &frac) {
        num.i = frac.num;
        denom.i = frac.num;
        _reduce();

        return *this;
    }
}
