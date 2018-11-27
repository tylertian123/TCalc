#ifndef __DEQUE_H__
#define __DEQUE_H__

#include "stm32f10x.h"
#include <stdlib.h>

template <typename T>
class Deque {
public:
    Deque(uint16_t capacity) : len(0), start(0), maxLen(capacity) {
        contents = (T*) malloc(sizeof(T) * capacity);
    }

    void enqueue(T elem) {
        uint16_t index = (start + len) % maxLen;
        ++len;
        contents[index] = elem;
    }
    T dequeue() {
        T &temp = contents[start];
        ++start;
        if(start >= maxLen) {
            start = 0;
        }
        return temp;
    }

protected:
    T *contents;
    uint16_t len;
    uint16_t start;
    uint16_t maxLen;
};

#endif
