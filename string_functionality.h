#ifndef STRING_FUNCTIONALITY_H
#define STRING_FUNCTIONALITY_H

// Include

#include <stddef.h>

// Data

#define KEY_MAX_LEN 255

// Public API (function protorypes)

int tokenize_string(const char* delimiter, char* input_str, int max_tokens, char** out_tokens);
unsigned char* ustrdup(const unsigned char *src);
int ustrcmp(const unsigned char* s1, const unsigned char* s2);
bool is_str_valid(const unsigned char* str);
size_t stosizet(const char* s);

#endif
