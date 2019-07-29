#include "numerical.hpp"
#include "util.hpp"
#include <cmath>

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
		if(divisor == 1 || divisor == 0) {
			return;
		}
		num.i /= divisor;
		denom.i /= divisor;
    }

    void Numerical::toDouble() {
        if(!isNumber()) {
            num.d = static_cast<double>(num.i) / denom.i;
            denom.i = IS_NUMBER_FLAG;
        }
    }

    void Numerical::toFraction() {
        if(isNumber() && isInt(num.d)) {
            num.i = num.d;
            denom.i = 1;
        }
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

    Numerical& Numerical::operator+=(double n) {
        // If n is an integer and this is a fraction, call the fraction += operator
        if(isInt(n) && (toFraction(), !isNumber())) {
            this->operator+=(Fraction(static_cast<int64_t>(n), 1));
        }
        // Otherwise convert to double and add normally
        else {
            toDouble();
            num.d += n;
        }

        return *this;
    }

    Numerical& Numerical::operator+=(const Fraction &frac) {
        // Try to convert to fraction first
        toFraction();
        // If not possible, convert arg to double and add normally
        if(isNumber()) {
            num.d += static_cast<double>(frac);
        }
        // Otherwise add fractions
        else {
            int64_t nd = lcm(denom.i, frac.denom);
            int64_t numA = num.i * (nd / denom.i);
            int64_t numB = frac.num * (nd / frac.denom);
            
            num.i = numA + numB;
            denom.i = nd;

            _reduce();
        }

		return *this;
    }

    Numerical& Numerical::operator+=(const Numerical &other) {
        return other.isNumber() ? this->operator+=(other.num.d) : this->operator+=(other.asFraction());
    }

    Numerical& Numerical::operator-=(double n) {
        // If n is an integer and this is a fraction, call the fraction -= operator
        if(isInt(n) && (toFraction(), !isNumber())) {
            this->operator-=(Fraction(static_cast<int64_t>(n), 1));
        }
        // Otherwise convert to double and subtract normally
        else {
            toDouble();
            num.d -= n;
        }

        return *this;
    }

    Numerical& Numerical::operator-=(const Fraction &frac) {
        // Try to convert to fraction first
        toFraction();
        // If not possible, convert arg to double and subtract normally
        if(isNumber()) {
            num.d -= static_cast<double>(frac);
        }
        // Otherwise subtract fractions
        else {
            int64_t nd = lcm(denom.i, frac.denom);
            int64_t numA = num.i * (nd / denom.i);
            int64_t numB = frac.num * (nd / frac.denom);
            
            num.i = numA - numB;
            denom.i = nd;

            _reduce();
        }

		return *this;
    }

    Numerical& Numerical::operator-=(const Numerical &other) {
        return other.isNumber() ? this->operator-=(other.num.d) : this->operator-=(other.asFraction());
    }

    Numerical& Numerical::operator*=(double n) {
        // If n is an integer and this is a fraction, call the fraction *= operator
        if(isInt(n) && (toFraction(), !isNumber())) {
            this->operator*=(Fraction(static_cast<int64_t>(n), 1));
        }
        // Otherwise convert to double and multiply normally
        else {
            toDouble();
            num.d *= n;
        }

        return *this;
    }

    Numerical& Numerical::operator*=(const Fraction &frac) {
        // Try to convert to fraction first
        toFraction();
        // If not possible, convert arg to double and multiply normally
        if(isNumber()) {
            num.d *= static_cast<double>(frac);
        }
        // Otherwise multiply fractions
        else {
            Fraction f(frac);
            int64_t r = gcd(num.i, f.denom);
            num.i /= r;
            f.denom /= r;
            r = gcd(denom.i, f.num);
            denom.i /= r;
            f.num /= r;

            num.i *= f.num;
            denom.i *= f.denom;

            _reduce();
        }

		return *this;
    }

    Numerical& Numerical::operator*=(const Numerical &other) {
        return other.isNumber() ? this->operator*=(other.num.d) : this->operator*=(other.asFraction());
    }

    Numerical& Numerical::operator/=(double n) {
        // If n is an integer and this is a fraction, call the fraction /= operator
        if(isInt(n) && (toFraction(), !isNumber())) {
            this->operator/=(Fraction(static_cast<int64_t>(n), 1));
        }
        // Otherwise convert to double and divide normally
        else {
            toDouble();
            num.d /= n;
        }

        return *this;
    }

    Numerical& Numerical::operator/=(const Fraction &frac) {
        // Try to convert to fraction first
        toFraction();
        // If not possible, convert arg to double and divide normally
        if(isNumber()) {
            num.d /= static_cast<double>(frac);
        }
        // Otherwise divide fractions
        else {
            // Multiply by inverse
            Fraction f(frac.denom, frac.num);
            int64_t r = gcd(num.i, f.denom);
            num.i /= r;
            f.denom /= r;
            r = gcd(denom.i, f.num);
            denom.i /= r;
            f.num /= r;

            num.i *= f.num;
            denom.i *= f.denom;

            _reduce();
        }

		return *this;
    }

    Numerical& Numerical::operator/=(const Numerical &other) {
        return other.isNumber() ? this->operator/=(other.num.d) : this->operator/=(other.asFraction());
    }

    Numerical Numerical::operator-() const {
        Numerical n(*this);
        if(n.isNumber()) {
            n.num.d = -n.num.d;
        }
        else {
            n.num.i = -n.num.i;
        }
        
        return n;
    }

    Numerical Numerical::operator+(double n) const {
        Numerical numerical(*this);
        numerical += n;
        
        return numerical;
    }

    Numerical Numerical::operator+(const Fraction &frac) const {
        Numerical n(*this);
        n += frac;

        return n;
    }

    Numerical Numerical::operator+(const Numerical &other) const {
        Numerical n(*this);
        n += other;

        return n;
    }

    Numerical operator+(double n, const Numerical &num) {
        return num + n;
    }

    Numerical operator+(const Fraction &frac, const Numerical &num) {
        return num + frac;
    }

    Numerical Numerical::operator-(double n) const {
        Numerical numerical(*this);
        numerical -= n;
        
        return numerical;
    }

    Numerical Numerical::operator-(const Fraction &frac) const {
        Numerical n(*this);
        n -= frac;

        return n;
    }

    Numerical Numerical::operator-(const Numerical &other) const {
        Numerical n(*this);
        n -= other;

        return n;
    }

    Numerical operator-(double n, const Numerical &num) {
        return -(num - n);
    }

    Numerical operator-(const Fraction &frac, const Numerical &num) {
        return -(num - frac);
    }

    Numerical Numerical::operator*(double n) const {
        Numerical numerical(*this);
        numerical *= n;
        
        return numerical;
    }

    Numerical Numerical::operator*(const Fraction &frac) const {
        Numerical n(*this);
        n *= frac;

        return n;
    }

    Numerical Numerical::operator*(const Numerical &other) const {
        Numerical n(*this);
        n *= other;

        return n;
    }

    Numerical operator*(double n, const Numerical &num) {
        return num * n;
    }

    Numerical operator*(const Fraction &frac, const Numerical &num) {
        return num * frac;
    }

    Numerical Numerical::operator/(double n) const {
        Numerical numerical(*this);
        numerical /= n;
        
        return numerical;
    }

    Numerical Numerical::operator/(const Fraction &frac) const {
        Numerical n(*this);
        n /= frac;

        return n;
    }

    Numerical Numerical::operator/(const Numerical &other) const {
        Numerical n(*this);
        n /= other;

        return n;
    }

    Numerical operator/(double n, const Numerical &num) {
        return Numerical(n) / num;
    }

    Numerical operator/(const Fraction &frac, const Numerical &num) {
        return Numerical(frac) / num;
    }

    bool Numerical::operator==(double n) const {
        return n == asDouble();
    }

    bool Numerical::operator==(const Fraction &frac) const {
        if(isNumber()) {
            return static_cast<double>(frac) == num.d;
        }
        else {
            Numerical n(frac);
            n._reduce();
            return n.num.i == num.i && n.denom.i == denom.i;
        }
    }

    bool Numerical::operator==(const Numerical &other) const {
        return other.isNumber() ? this->operator==(other.num.d) : this->operator==(other.asFraction());
    }

    bool operator==(double n, const Numerical &num) {
        return num == n;
    }

    bool operator==(const Fraction &frac, const Numerical &num) {
        return num == frac;
    }

    bool Numerical::operator!=(double d) const {
        return !this->operator==(d);
    }

    bool Numerical::operator!=(const Fraction &frac) const {
        return !this->operator==(frac);
    }

    bool Numerical::operator!=(const Numerical &other) const {
        return !this->operator==(other);
    }

    bool operator!=(double n, const Numerical &num) {
        return !(num == n);
    }

    bool operator!=(const Fraction &frac, const Numerical &num) {
        return !(num == frac);
    }

    Numerical::operator double() const {
        return asDouble();
    }

    Numerical::operator util::Fraction() const {
        return asFraction();
    }

    void Numerical::sqrt() {
        if(!isNumber()) {
            // Try to take the square root of the numerator and denominator
            double rootN = std::sqrt(static_cast<double>(num.i));
            // Continue if the numerator turned out to be an integer
            if(isInt(rootN)) {
                double rootD = std::sqrt(static_cast<double>(denom.i));
                if(isInt(rootD)) {
                    // The square root of a reduced fraction is always reduced
                    num.i = static_cast<int64_t>(rootN);
                    denom.i = static_cast<int64_t>(rootD);
                    // Return here
                    return;
                }
            }
        }
        // If the function didn't already return, then taking the square root as a fraction failed
        // Take the square root normally as a number
        toDouble();
        num.d = std::sqrt(num.d);
    }

    bool Numerical::feq(double n) const {
        return floatEq(n, asDouble());
    }

    bool Numerical::feq(const Fraction &frac) const {
        if(isNumber()) {
            return floatEq(static_cast<double>(frac), num.d);
        }
        else {
            Numerical n(frac);
            n._reduce();
            return n.num.i == num.i && n.denom.i == denom.i;
        }
    }

    bool Numerical::feq(const Numerical &other) const {
        return other.isNumber() ? feq(other.asDouble()) : feq(other.asFraction());
    }
}
