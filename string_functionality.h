// Include

#ifndef STRING_FUNCTIONALITY_H
#define STRING_FUNCTIONALITY_H

// Data

#define KEY_MAX_LEN 255

// Public API (function protorypes)

int tokenize_string(const char* delimiter, char* input_str, int max_tokens, char** out_tokens);
unsigned char *ustrdup(const unsigned char *src);
int ustrcmp(const unsigned char* s1, const unsigned char* s2);
int is_key_valid(const unsigned char* key);
unsigned char* key_formatter(const char* input_key);

#endif
