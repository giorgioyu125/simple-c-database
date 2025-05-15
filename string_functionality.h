#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// Strings functionality

unsigned char *ustrdup(const unsigned char *src) {
    if (src == NULL) {
        return NULL;
    }

    size_t len = strlen((const char *)src);
    unsigned char *dest = (unsigned char *)malloc((len + 1) * sizeof(unsigned char));

    if (dest == NULL) {
        return NULL;
    }

    memcpy(dest, src, len + 1);
    return dest;
}

int ustrcmp(const unsigned char* s1, const unsigned char* s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (int)(*s1) - (int)(*s2);
}


