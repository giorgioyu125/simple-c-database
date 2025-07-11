#include "bitwise_functionality.h"

// Helper functions

size_t next_power_of_2(size_t n) {
    if (n == 0) {
        return 1;
    }

    n--;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    #if defined(__x86_64__) || defined(_M_X64)
    n |= n >> 32;
    #endif

    n++;

    return n;
}
