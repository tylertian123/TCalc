#ifndef __UTIL_H__
#define __UTIL_H__

#include <math.h>

//Execute method on obj with arguments if obj is not null
#define SAFE_EXEC(obj, method, ...) if(obj) (obj)->method(__VA_ARGS__)
//Execute method on obj with arguments and return the result if obj is not null, otherwise results in 0
#define SAFE_EXEC_0(obj, method, ...) ((obj) ? ((obj)->method(__VA_ARGS__)) : 0)
//Access field of obj if obj is not null, otherwise results in 0
#define SAFE_ACCESS_0(obj, field) ((obj) ? ((obj)->field) : 0)

#include "stm32f10x.h"
#define __NO_INTERRUPT(x) \
		uint32_t __no_interrupt_PRIMASK=__get_PRIMASK();__disable_irq();\
		x \
		if(!__no_interrupt_PRIMASK)__enable_irq()

template <typename T>
inline const T& max(const T &a, const T &b) {
	return b > a ? b : a;
}
template <typename T>
inline const T& min(const T &a, const T &b) {
	return b < a ? b : a;
}
template <typename T>
inline T abs(T x) {
	return x >= 0 ? x : -x;
}
template <typename T>
inline T positiveMod(T a, T b) {
	return a % b + (a < 0 ? b : 0);
}
template <typename T>
inline T floorDiv(T a, T b) {
	return a > 0 ? a / b : (a - b + 1) / b;
}
template <typename T>
inline bool isInt(T n) {
	return static_cast<int64_t>(n) == n;
}
template <typename T>
void swap(T &t1, T &t2) {
	T tmp(t1);
	t1 = t2;
	t2 = tmp;
}

inline double round(double d, int16_t decimals) {
	double p = pow(10.0, decimals);
	return ::round(d * p) / p;
}

#endif
