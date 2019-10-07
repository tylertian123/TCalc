#include "ntoa.hpp"
#include <stdio.h>
#include <math.h>
#include "lcd12864_charset.hpp"

namespace util {

    // Reverses a string
    void _ntoa_rev(char *a, uint8_t len) {
        for (uint8_t i = 0; i < len / 2; ++i) {
            char temp = a[i];
            a[i] = a[len - 1 - i];
            a[len - 1 - i] = temp;
        }
    }

    uint8_t ltoa(int64_t val, char *str) {
        uint8_t len = 0;
        bool neg = val < 0;

        // Handle zero and negatives
        if (val == 0) {
            str[len++] = '0';
        }
        else if (neg) {
            str[len++] = '-';
            val = -val;
        }
        while (val) {
            // Write out each digit in reverse
            str[len++] = val % 10 + '0';
            val /= 10;
        }

        // Reverse the string
        _ntoa_rev(neg ? str + 1 : str, neg ? len - 1 : len);

        str[len] = '\0';

        return len;
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
