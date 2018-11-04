#ifndef __DYNAMIC_ARRAY_H__
#define __DYNAMIC_ARRAY_H__

#include "stm32f10x.h"
#include <stdlib.h>

template <typename T>
class DynamicArray {
public:
	//Default constructor with length 0
	DynamicArray() : contents(nullptr), len(0), maxLen(0) {}
	//Constructor with initial capacity
	DynamicArray(uint32_t initialCapacity) : len(0), maxLen(initialCapacity) {
		//Make sure the length is multipled by the size of T
		contents = (T*) malloc(sizeof(T) * initialCapacity);
	}
	//Copy constructor
	DynamicArray(const DynamicArray &other) : len(other.len), maxLen(other.maxLen) {
		contents = (T*) malloc(sizeof(T) * maxLen);
		
		for(uint32_t i = 0; i < len; i ++) {
			contents[i] = other.contents[i];
		}
	}
	//Array constructor
	DynamicArray(const T *arr, uint32_t len) : len(len), maxLen(len) {
		contents = (T*) malloc(sizeof(T) * maxLen);
		
		for(uint32_t i = 0; i < len; i ++) {
			contents[i] = arr[i];
		}
	}
	~DynamicArray() {
		free(contents);
	}
	
	uint32_t length() {
		return len;
	}
	uint32_t maxLength() {
		return maxLen;
	}
	void resize(uint32_t newSize) {
		//Ignore if the new size is less than the length
		if(newSize < len) {
			return;
		}
		//Otherwise reallocate memory
		maxLen = newSize;
		//Make sure the length is multipled by the size of T
		realloc(contents, sizeof(T) * newSize);
	}
	void minimize() {
		resize(len);
	}
	
	void add(const T &elem) {
		len ++;
		//If the new length is more than what we can store then reallocate
		if(len > maxLen) {
			//Default implementation: reallocate only what's needed
			maxLen = len;
			realloc(contents, sizeof(T) * len);
		}
		contents[len - 1] = elem;
	}
	void removeLast() {
		if(len == 0) {
			//Ignore if there are no elements
			return;
		}
		//Simply decrement the length, no need to waste time clearing out the memory
		len --;
	}
	void empty() {
		len = 0;
	}
	
	//WARNING: Does not check for out of bounds!
	const T& operator[](uint32_t i) const {
		return contents[i];
	}
	T& operator[](uint32_t i) {
		return contents[i];
	}
	DynamicArray& operator+=(const T &elem) {
		add(elem);
		return *this;
	}
	

protected:
	T *contents;
	uint32_t len;
	uint32_t maxLen;
};

#endif
