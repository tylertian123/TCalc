#ifndef __DYNAMIC_ARRAY_H__
#define __DYNAMIC_ARRAY_H__

#include "stm32f10x.h"
#include <stdlib.h>

template <typename T>
class DynamicArray {
public:
	//Default constructor with length 0
	DynamicArray() : contents((T*) malloc(0)), len(0), maxLen(0) {}
	//Constructor with initial capacity
	DynamicArray(uint16_t initialCapacity) : len(0), maxLen(initialCapacity) {
		//Make sure the length is multipled by the size of T
		contents = (T*) malloc(sizeof(T) * initialCapacity);
	}
	//Copy constructor
	DynamicArray(const DynamicArray &other) : len(other.len), maxLen(other.maxLen) {
		contents = (T*) malloc(sizeof(T) * maxLen);
		
		for(uint16_t i = 0; i < len; i ++) {
			contents[i] = other.contents[i];
		}
    }
	typedef T* iterator;
	typedef const T* const_iterator;
    //Iterator constructor
    DynamicArray(const_iterator start, const_iterator fin) : len(fin - start), maxLen(fin - start) {
        contents = (T*) malloc(sizeof(T) * maxLen);

        for(iterator i = begin(); i != end(); i ++) {
            //Go through every elem and initialize its value
            *i = *(start ++);
        }
    }
	//Array constructor
	DynamicArray(const T *arr, uint16_t len) : len(len), maxLen(len) {
		contents = (T*) malloc(sizeof(T) * maxLen);
		
		for(uint16_t i = 0; i < len; i ++) {
			contents[i] = arr[i];
		}
	}
	~DynamicArray() {
		free(contents);
	}
	
	uint16_t length() {
		return len;
	}
	uint16_t maxLength() {
		return maxLen;
	}
	bool resize(uint16_t newSize) {
        uint16_t oldSize = maxLen;
		//Ignore if the new size is less than the length
		if(newSize < len) {
			return true;
		}
		//Otherwise reallocate memory
		maxLen = newSize;
		//Make sure the length is multipled by the size of T
		void *tmp = realloc(contents, sizeof(T) * newSize);
        //Oh crap we ran out of memory
        if(!tmp) {
            //Reset max len
            maxLen = oldSize;
            return false;
        }
        contents = (T*) tmp;
        return true;
	}
	void minimize() {
		resize(len);
	}
	
	bool add(const T &elem) {
		len ++;
		//If the new length is more than what we can store then reallocate
		if(len > maxLen) {
			//Default implementation: reallocate only what's needed
			maxLen = len;
			void *tmp = realloc(contents, sizeof(T) * len);
            if(!tmp) {
                //Oh crap we ran out of memory, have a panic attack here
                len --;
                maxLen --;
                return false;
            }
            contents = (T*) tmp;
		}
		contents[len - 1] = elem;
        return true;
	}
    bool insert(const T &elem, uint16_t where) {
        len ++;
        if(len > maxLen) {
            //Default impl: reallocate only what's needed
            maxLen = len;
            void *tmp = realloc(contents, sizeof(T) * len);
            if(!tmp) {
                //Oh crap we ran out of memory
                len --;
                maxLen --;
                return false;
            }
            contents = (T*) tmp;
        }
        //Iterate backwards to move the elements
        //This way we don't have to keep a buffer
        for(uint16_t i = len - 1; i > where; i --) {
            contents[i] = contents[i - 1];
        }
        contents[where] = elem;
        return true;
    }
    bool insert(const T elem[], uint16_t count, uint16_t where) {
        len += count;
        if(len > maxLen) {
            uint16_t oldMaxLen = maxLen;
            maxLen = len;
            void *tmp = realloc(contents, sizeof(T) * len);
            if(!tmp) {
                len -= count;
                maxLen = oldMaxLen;
                return false;
            }
            contents = (T*) tmp;
        }
        //Iterate backwards to move the elements
        //This way we don't have to keep a buffer
        for(uint16_t i = len - 1; i > where; i --) {
            contents[i] = contents[i - count];
        }
        for(uint16_t i = where; i < where + count; i ++) {
            contents[i] = elem[i - where];
        }
        return true;
    }
    void removeAt(uint16_t where) {
        //Ignore if out of bounds
        if(where >= len) {
            return;
        }
        len --;
        //Shift all elements after the index back
        for(uint16_t i = where; i < len; i ++) {
            contents[i] = contents[i + 1];
        }
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
	const T& operator[](uint16_t i) const {
		return contents[i];
	}
	T& operator[](uint16_t i) {
		return contents[i];
	}
	DynamicArray& operator+=(const T &elem) {
		add(elem);
		return *this;
	}
	
	iterator begin() {
		return &contents[0];
	}
	const_iterator begin() const {
		return &contents[0];
	}
	iterator end() {
		return &contents[len];
	}
	const_iterator end() const {
		return &contents[len];
	}

protected:
	T *contents;
	uint16_t len;
	uint16_t maxLen;
};

#endif
