#include "numerical.hpp"
#include "util.hpp"

namespace util {

    Numerical::Numerical() {
        num.d = 0;
        // Mark as number
        denom.i = 0x8000'0000'0000'0000;
    }
    Numerical::Numerical(double val) {
        num.d = val;
        // Mark as number
        denom.i = 0x8000'0000'0000'0000;
    }
    Numerical::Numerical(int64_t num, int64_t denom) {
        this->num.i = num;
        this->denom.i = denom;
        // Reduce the fraction without checking for validity
        _reduce();
    }

    Numerical::Numerical(const Numerical &other) {
        num = other.num;
        denom = other.denom;
    }
    Numerical::Numerical(Numerical &&other) {
        num = other.num;
        denom = other.denom;
    }

    bool Numerical::isNumber() const {
        // See docs for Numerical::DoubleOrInt64
        return denom.i >> 63;
    }

    double Numerical::asDouble() const {
        if(isNumber()) {
            return num.d;
        }
        else {
            return static_cast<double>(num.i) / denom.i;
        }
    }

    int64_t Numerical::numerator() const {
        return num.i;
    }

    int64_t Numerical::denominator() const {
        return denom.i;
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
}
