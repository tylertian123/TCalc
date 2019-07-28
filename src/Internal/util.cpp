#include "util.hpp"
#include <float.h>

namespace util {

    bool floatEq(double a, double b, double epsilon) {
        if(!isfinite(a) || !isfinite(b)) {
            return false;
        }
        if(a == b) {
            return true;
        }
        const double absA = abs(a);
        const double absB = abs(b);
        const double diff = abs(a - b);
        if(a == 0 || b == 0 || diff < DBL_EPSILON) {
            return diff < (epsilon * DBL_EPSILON);
        }
        else {
            return diff / (absA + absB) < epsilon;
        }
    }

    int64_t gcd(int64_t a, int64_t b) {
		while(true) {
			if(a == 0) {
				return b;
			}
			if(b == 0) {
				return a;
			}

			int64_t r = a % b;
			a = b;
			b = r;
		}
	}
    
	int64_t lcm(int64_t a, int64_t b) {
		return (a * b) / gcd(a, b);
	}
}
