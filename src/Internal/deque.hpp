#ifndef __DEQUE_H__
#define __DEQUE_H__

#include "stm32f10x.h"
#include <stdlib.h>

namespace util {
    template <typename T, uint16_t IncreaseAmount = 8>
    class Deque {
    public:
        Deque(uint16_t capacity) : len(0), start(0), maxLen(capacity) {
            contents = (T*) malloc(sizeof(T) * capacity);
        }
        Deque() : contents((T*) malloc(0)), len(0), start(0), maxLen(0) {}
        ~Deque() {
            free(contents);
        }

        bool increaseSize(uint16_t increase) {
            uint16_t oldMaxLen = maxLen;
            maxLen = len + increase;
            void *tmp = realloc(contents, sizeof(T) * maxLen);
            if(!tmp) {
                maxLen = oldMaxLen;
                return false;
            }
            // Adjust the queue to accommodate for the change
            contents = (T*) tmp;
            uint16_t i = maxLen - 1;
            uint16_t index = (start + len) % maxLen;
            while(i != index) {
                if(i + increase >= maxLen) {
                    contents[i] = contents[(i + increase) % maxLen];
                }
                else {
                    contents[i] = contents[i + increase];
                }
                ++i;
                if(i >= maxLen) {
                    i = 0;
                }
            }
            return true;
        }

        bool enqueue(T elem) {
            // Check for overflow
            if(len + 1 > maxLen) {
                if(!increaseSize(IncreaseAmount)) {
                    return false;
                }
            }
            uint16_t index = (start + len) % maxLen;
            contents[index] = elem;
            ++len;
            return true;
        }
        bool push(T elem) {
            // Check for overflow
            if(len + 1 > maxLen) {
                if(!increaseSize(IncreaseAmount)) {
                    return false;
                }
            }

            if(start > 0) {
                --start;
            }
            else {
                start = maxLen - 1;
            }
            contents[start] = elem;
            ++len;
            return true;
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
        T peek() const {
            return contents[start];
        }
        bool isEmpty() const {
            return !(len > 0);
        }
        uint16_t length() const {
            return len;
        }

    protected:
        T *contents;
        uint16_t len;
        uint16_t start;
        uint16_t maxLen;
    };
}

#endif
