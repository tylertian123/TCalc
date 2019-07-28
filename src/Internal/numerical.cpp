#include "numerical.hpp"

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
}
