#include "ntoa.hpp"
#include <math.h>
#include <string.h>
#include "util.hpp"

//Reverses a string
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

    //Handle zero and negatives
	if (val == 0) {
		str[len++] = '0';
	}
	else if (neg) {
		str[len++] = '-';
		val = -val;
	}
	while (val) {
        //Write out each digit in reverse
		str[len++] = val % 10 + '0';
		val /= 10;
	}

    //Reverse the string
	_ntoa_rev(neg ? str + 1 : str, neg ? len - 1 : len);

	str[len] = '\0';

	return len;
}

//Converts double to ASCII string
//ndigits is the number of significant digits
//echar is the character to use to represent 10^x in the case of scientific notation, e.g. 2.34e10
uint8_t ftoa(double val, char *str, uint8_t ndigits, char echar) {
    //Special cases
    if(isnan(val)) {
        strcpy(str, "NaN");
        return 3;
    }
    else if(val == INFINITY) {
        strcpy(str, "Infinity");
        return 8;
    }
    else if(val == -INFINITY) {
        strcpy(str, "-Infinity");
        return 9;
    }
    else if(val == 0) {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }
    //Handle values greater or equal to 10^6 or less than or equal to 10^-5 in scientific notation
    if(abs(val) >= 1e6) {
        //Divide until the number is less than 10
        uint16_t p = 0;
        while(abs(val) >= 10) {
            val /= 10;
            ++p;
        }
        //Convert the new number
        uint8_t len = ftoa(val, str, ndigits, echar);
        str[len++] = echar;
        len += ltoa(p, str + len);
        return len;
    } 
    else if(abs(val) <= 1e-5) {
        uint16_t p = 0;
        while(abs(val) < 1) {
            val *= 10;
            ++p;
        }
        uint8_t len = ftoa(val, str, ndigits, echar);
        str[len++] = echar;
        //Negate p since we multiplied instead of divided
        len += ltoa(-p, str + len);
        return len;
    }

    int64_t whole = (int64_t) val;
    uint8_t len = ltoa(whole, str);
    //If it's an integer, finish here
    if(whole == val) {
        return len;
    }

    if(len < ndigits) {
        str[len++] = '.';
        double frac = val - whole;
        //Move the decimal point and round
        int64_t nfrac = round(frac * powl(10, ndigits - len));
        if(nfrac < 0) {
            nfrac *= -1;
        }
        char buf[64];
        uint8_t fracLen = ltoa(nfrac, buf);
        
        uint8_t diff = ndigits - len - fracLen;
        while(diff--) {
            str[len++] = '0';
        }
        uint8_t index = 0;
        while(fracLen--) {
            str[len++] = buf[index++];
        }

        //finally, shave off any unneeded trailing zeros
        while(str[len - 1] == '0') {
            --len;
        }
        if(str[len - 1] == '.') {
            --len;
        }
        str[len] = '\0';
    }
    //null-termination is handled by ltoa
    
    return len;
}
