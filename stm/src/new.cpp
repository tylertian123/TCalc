#include <malloc.h>
#include <new>

void *operator new(std::size_t size) {
    return malloc(size);
}

void *operator new[](std::size_t size) {
    return malloc(size);
}

void operator delete(void *ptr) {
    free(ptr);
}

void operator delete[](void *ptr) {
    free(ptr);
}

/* Optionally you can override the 'nothrow' versions as well.
   This is useful if you want to catch failed allocs with your
   own debug code, or keep track of heap usage for example,
   rather than just eliminate exceptions.
 */

void *operator new(std::size_t size, const std::nothrow_t &) {
    return malloc(size);
}

void *operator new[](std::size_t size, const std::nothrow_t &) {
    return malloc(size);
}

void operator delete(void *ptr, const std::nothrow_t &) {
    free(ptr);
}

void operator delete[](void *ptr, const std::nothrow_t &) {
    free(ptr);
}
