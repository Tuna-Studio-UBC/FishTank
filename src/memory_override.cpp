#include <cstdlib>
#include <tracy/Tracy.hpp>

// https://stackoverflow.com/questions/73218117/are-there-any-tools-can-record-the-call-tree-of-each-memory-allocation-in-window#:~:text=I've%20been%20using%20Tracy%20profiler%20to%20track,the%20operator%20new%20and%20operator%20delete%20though.

void *operator new(size_t size) {
    void *ptr = malloc(size);
    TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}

void *operator new[](size_t size) {
    void *ptr = malloc(size);
    TracyAlloc(ptr, size);
    return ptr;
}

void operator delete[](void *ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}