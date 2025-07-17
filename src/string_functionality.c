// Header
#include "string_functionality.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


// Safe Conversion

size_t stosizet(const char* s){
    if (s == NULL){
        return 0;
    }

    if (*s == '\0'){
        return 0;
    }

    size_t result = 0;
    for (; *s != '\0'; ++s){
        char c = *s;
        if (c < '0' || c > '9'){
            return 0; 
        }
        int digit = c - '0';
        if (result > (SIZE_MAX - (size_t)digit) / 10){
            return 0; 
        }
        result = result * 10 + (size_t)digit;
    }

    return result;
}

unsigned int sizet_to_uint(size_t s, bool* error_flag){
    if (error_flag == NULL){
        return 0;
    }

    *error_flag = false;

    if (s > UINT_MAX){
        *error_flag = true;
        return 0;
    }

    return (unsigned int)s;
}

size_t ssizet_to_sizet(ssize_t input_value, bool* error_flag){
    if (error_flag == NULL){
        return 0;
    }

    *error_flag = false;

    if (input_value < 0){
        *error_flag = true;
        return 0;
    }

    return (size_t)input_value;
}

int long_to_int(long l, bool* error){
    if (error == NULL){

        return 0;
    }

    if (l > INT_MAX){
        *error = true;
        return 0;
    }

    if (l < INT_MIN){
        *error = true;
        return 0;
    }

    *error = false;
    return (int)l;
}

size_t long_to_sizet(long l, bool* error){

    if (error == NULL) return 0; 

    if (l < 0){
        *error = true;
        return 0;
    }

    if ((unsigned long)l > SIZE_MAX){
        *error = true;
        return 0;
    }

    *error = false;
    return (size_t)l;
}

unsigned long int_to_ul(int i, bool* error){
    if (error == NULL) return 0;

    if (i < 0){
        *error = true;
        return 0;
    }

    *error = false;
    return (unsigned long)i;
}

int sizet_to_int(size_t s, bool* error){
    if (error == NULL) return 0;

    if (s > INT_MAX){
        *error = true;
        return 0;
    }

    *error = false;
    return (int)s;
}

// Memory Functionality

void* memdup(const void* src, size_t len){
    void* dest = malloc(len);
    if (dest == NULL){
        return NULL; 
    }

    return memcpy(dest, src, len);
}

// Strings Functionality

int ustrncmp(const unsigned char *s1, const unsigned char *s2, size_t n){
    if (n == 0) {
        return 0;
    }

    while ((n-- > 0) && (*s1 == *s2)) {

        if (*s1 == '\0') {
            return 0;
        }
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int tokenize_string(const char* delimiter, char* input_str, int max_tokens, char** out_tokens){
    int count = 0;

    if (delimiter == NULL){
        return -1;
    }

    char* token = strtok(input_str, delimiter);

    while (token != NULL){
        if (count >= max_tokens){
            break;
        }

        out_tokens[count] = token;
        count++;

        token = strtok(NULL, delimiter);
    }

    return count;
}

unsigned char* ustrncpy(unsigned char* dest, const unsigned char* src, size_t n){
    if ((dest == NULL) || (src == NULL) || (n == 0)) {
        return dest;
    }
    
    size_t i;
    for (i = 0; (i < n - 1) && (src[i] != '\0'); i++) {
        dest[i] = src[i];
    }

    dest[i] = '\0';

    return dest;
}

size_t ustrlen(const unsigned char* str){

    if (str == NULL){
        return 0;
    }

    const unsigned char* start = str;

    while (*str != '\0'){
        str++;
    }

    return (size_t)(str - start);
}

unsigned char* ustrdup(const char *src){
    if (src == NULL){
        return NULL;
    }

    size_t len = strlen((const char *)src);
    unsigned char *dest = (unsigned char *)malloc((len + 1) * sizeof(unsigned char));

    if (dest == NULL){
        return NULL;
    }

    memcpy(dest, src, len + 1);
    return dest;
}

int ustrcmp(const unsigned char* s1, const unsigned char* s2){
    while (*s1 != '\0' && *s1 == *s2){
        s1++;
        s2++;
    }

    return (int)(*s1) - (int)(*s2);
}

ssize_t strscpy(char* d, const char* s, size_t n){
    size_t i;

    for (i = 0; i < n; i++)
        if ('\0' == (d[i] = s[i]))
            return i > SSIZE_MAX ? -1 : (ssize_t) i;

    if (i == 0)
        return 0;

    d[--i] = '\0';
    return -1;
}

bool is_key_valid(const unsigned char* key){
    return ((key != NULL) && (key[0] != '\0'));
}
