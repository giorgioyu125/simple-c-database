#ifndef STRING_FUNCTIONALITY_H
#define STRING_FUNCTIONALITY_H

// Includes

#include <stddef.h> 
#include <stdbool.h> 

// Public API
    int tokenize_string(const char* delimiter, char* input_str, int max_tokens, char** out_tokens);
    size_t stosizet(const char* s);
    size_t ustrlen(const unsigned char* str);
    unsigned char* ustrdup(const char *src);
    int ustrcmp(const unsigned char* s1, const unsigned char* s2);
    bool is_key_valid(const unsigned char* key);

#endif 
