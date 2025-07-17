#ifndef STRING_FUNCTIONALITY_H
#define STRING_FUNCTIONALITY_H

// Includes

#include <stddef.h> 
#include <stdbool.h> 
#include <stdio.h>

// Macro and Defines

#define KEY_MAX_LEN 255

// Public API
    
    void* memdup(const void* src, size_t len);
    int long_to_int(long l, bool* error);
    size_t stosizet(const char* s);
    size_t long_to_sizet(long l, bool* error);
    unsigned long int_to_ul(int i, bool* error);
    int sizet_to_int(size_t s, bool* error);
    unsigned int sizet_to_uint(size_t s, bool* error);
    size_t ssizet_to_sizet(ssize_t s, bool* error);

    
    unsigned char* ustrncpy(unsigned char* dest, const unsigned char* src, size_t n);
    int ustrncmp(const unsigned char *s1, const unsigned char *s2, size_t n);
    int tokenize_string(const char* delimiter, char* input_str, int max_tokens, char** out_tokens);
    size_t ustrlen(const unsigned char* str);
    unsigned char* ustrdup(const char *src);
    int ustrcmp(const unsigned char* s1, const unsigned char* s2);
    bool is_key_valid(const unsigned char* key);


#endif 
