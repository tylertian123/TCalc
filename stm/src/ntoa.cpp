#include "ntoa.hpp"
#include <stdio.h>
#include <math.h>
#include "lcd12864_charset.hpp"

namespace util {

    uint8_t ltoa(int64_t val, char *str) {
        return sprintf(str, "%lld", val);
    }

    // Converts double to ASCII string
    // ndigits is the number of significant digits
    // echar is the character to use to represent 10^x in the case of scientific notation, e.g. 2.34e10
    uint8_t ftoa(double val, char *str, uint8_t ndigits, char echar) {
        if(isnan(val)) {
            str[0] = '\xff';
            str[1] = '\0';
            return 1;
        }
        uint8_t len = sprintf(str, "%.*g", ndigits, val);
        for(; *str != '\0'; str++) {
            if(*str == 'e') {
                *str = echar;
            }
        }
        return len;
    }
}
