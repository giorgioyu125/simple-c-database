// Header
#include "string_functionality.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Strings functionality

int tokenize_string(const char* delimiter, char* input_str, int max_tokens, char** out_tokens) {
    int count = 0;

    if (delimiter == NULL){
        return -1;
    }

    char* token = strtok(input_str, delimiter);

    while (token != NULL) {
        if (count >= max_tokens) {
            break;
        }

        out_tokens[count] = token;
        count++;

        token = strtok(NULL, delimiter);
    }

    return count;
}

size_t stosizet(const char* s) {
    if (s == NULL) {
        return 0;
    }

    if (*s == '\0') {
        return 0;
    }

    size_t result = 0;
    for (; *s != '\0'; ++s) {
        char c = *s;
        if (c < '0' || c > '9') {
            return 0; 
        }
        int digit = c - '0';
        if (result > (SIZE_MAX - (size_t)digit) / 10) {
            return 0; 
        }
        result = result * 10 + (size_t)digit;
    }

    return result;
}

size_t ustrlen(const unsigned char* str){

    if (str == NULL) {
        return 0;
    }

    const unsigned char* start = str;

    while (*str != '\0') {
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

bool is_key_valid(const unsigned char* key){
    return ((key != NULL) && (key[0] != '\0'));
}
