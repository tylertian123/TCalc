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

    bool enqueue(T elem) {
        //Check for overflow
        if(len + 1 > maxLen) {
            uint16_t oldMaxLen = maxLen;
            //Default: allocate only what's needed
            maxLen = len + 1;
            void *tmp = realloc(contents, sizeof(T) * maxLen);
            if(!tmp) {
                maxLen = oldMaxLen;
                return false;
            }
            //Adjust the queue to accommodate for the change
            contents = (T*) tmp;
            uint16_t i = maxLen - 1;
            uint16_t index = (start + len) % maxLen;
            while(i != index) {
                if(i == maxLen - 1) {
                    contents[i] = contents[0];
                }
                else {
                    contents[i] = contents[i + 1];
                }
            }
        }
        uint16_t index = (start + len) % maxLen;
        contents[index] = elem;
        ++len;
        return true;
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
        --len;
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
