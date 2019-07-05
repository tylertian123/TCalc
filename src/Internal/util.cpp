#include "util.hpp"
#include <float.h>

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
