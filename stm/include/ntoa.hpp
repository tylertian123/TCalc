#ifndef __NTOA_H__
#define __NTOA_H__

#include "stm32f10x.h"

namespace util {
    // Reverses a string
    inline void _ntoa_rev(char *a, uint8_t len) {
        for (uint8_t i = 0; i < len / 2; ++i) {
            char temp = a[i];
            a[i] = a[len - 1 - i];
            a[len - 1 - i] = temp;
        }
    }

    template <typename T>
    uint8_t dtoa(T val, char *str) {
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

    uint8_t ftoa(double d, char *buf, uint8_t ndigits, char echar = 'e');
}

#endif
