#ifndef __UTIL_H__
#define __UTIL_H__

//Execute method on obj with arguments if obj is not null
#define SAFE_EXEC(obj, method, ...) if(obj) (obj)->method(__VA_ARGS__)
//Execute method on obj with arguments and return the result if obj is not null, otherwise results in 0
#define SAFE_EXEC_0(obj, method, ...) ((obj) ? ((obj)->method(__VA_ARGS__)) : 0)
//Access field of obj if obj is not null, otherwise results in 0
#define SAFE_ACCESS_0(obj, field) ((obj) ? ((obj)->field) : 0)

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
