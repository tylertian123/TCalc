#ifndef __UTIL_H__
#define __UTIL_H__

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

#endif
