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
    ~Deque() {
        free(contents);
    }

    void enqueue(T elem) {
        uint16_t index = (start + len) % maxLen;
        ++len;
        contents[index] = elem;
    }
    void push(T elem) {
        if(start > 0) {
            --start;
        }
        else {
            start = maxLen - 1;
        }
        contents[start] = elem;
    }
    T dequeue() {
        T &temp = contents[start];
        ++start;
        if(start >= maxLen) {
            start = 0;
        }
        return temp;
    }
    T pop() {
        return dequeue();
    }
    T peek() {
        return contents[start];
    }

protected:
    T *contents;
    uint16_t len;
    uint16_t start;
    uint16_t maxLen;
};

#endif
