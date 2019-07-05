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

//Converts double to ASCII string
//ndigits is the number of significant digits
//echar is the character to use to represent 10^x in the case of scientific notation, e.g. 2.34e10
uint8_t ftoa(double val, char *str, uint8_t ndigits, char echar) {
	// Special cases
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
	// Determine last nonzero digit for values greater than 1
	uint8_t lastNonzero = 0;
	if(val > 1) {
		double val2 = val;
		// Check if the last digit is 0
		while(fmod(val2, 10) == 0) {
			++lastNonzero;
			// If the last nonzero digit has more than 5 zeros after it, then we know we must display it in scientific notation
			if(lastNonzero > 5) {
				break;
			}
			val2 /= 10;
		}
	}

	// Scientific notation handling
	if(lastNonzero > 5) {
		// Divide until the number is less than 10
		uint16_t p = 0;
		while(abs(val) >= 10) {
			val /= 10;
			++p;
		}
		// Convert the new number
		uint8_t len = ftoa(val, str, ndigits, echar);
		str[len++] = echar;
		len += ltoa(p, str + len);
		return len;
	} 
	else if(abs(val) <= 1e-5) {
		uint16_t p = 0;
		// Multiply until the number is greater than 1
		while(abs(val) < 1) {
			val *= 10;
			++p;
		}
		uint8_t len = ftoa(val, str, ndigits, echar);
		str[len++] = echar;
		// Negate p since we multiplied instead of divided
		len += ltoa(-p, str + len);
		return len;
	}

	// Keep track of the length of the string
	uint8_t len = 0;
	// Isolate whole part
	uint64_t whole = abs((int64_t) val);
	// Add negative sign if necessary
	if(val < 0) {
		str[len++] = '-';
	}
	// Keep track of the digits in the whole part
	uint8_t wholeDigits = ltoa(whole, str + len);
	len += wholeDigits;
	// If it's an integer, finish here
	if (whole == abs(val)) {
		return len;
	}

	// Otherwise if the amount of significant digits is greater than what was in the whole part
	if(wholeDigits < ndigits) {
		str[len++] = '.';
		// Isolate fractional part
		// Make sure the absolute value is taken so that |frac| < 1 for all cases
		double frac = abs(val) - whole;
		// Move the decimal point and round
		double mult = pow(10.0, ndigits - wholeDigits);
		int64_t nfrac = round(frac * mult);
		if (nfrac < 0) {
			nfrac *= -1;
		}
		// If the fractional part turns out to be the same as the multiplier then it rounds to 1
		if (nfrac == mult) {
			// Take the whole part of the original value and increment its absolute value by 1
			int64_t rval = val;
			rval += (val > 0 ? 1 : -1);
			// Call ltoa on it and return the result
			return ltoa(rval, str);
		}
		char buf[64];
		// Convert fractional part
		uint8_t fracLen = ltoa(nfrac, buf);
		
		// Add any zeros missed by ltoa
		// ndigits should equal wholeDigits + fracLen precisely if no digits were missed
		// Otherwise the difference is calculated here
		uint8_t diff = ndigits - wholeDigits - fracLen;
		while(diff--) {
			str[len++] = '0';
		}
		// After inserting the zeros copy the fractional part
		uint8_t index = 0;
		while(fracLen--) {
			str[len++] = buf[index++];
		}

		// finally, shave off any unneeded trailing zeros
		while(str[len - 1] == '0') {
			--len;
		}
		if(str[len - 1] == '.') {
			--len;
		}
		str[len] = '\0';
	}
	
	return len;
}
