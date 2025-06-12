// Header
#include "hashing_functionality.h"

// Hashing functionality

unsigned long hash(const unsigned char *str) {
    unsigned long hash = 5381;
    unsigned long c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash;
}

