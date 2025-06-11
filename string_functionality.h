// Include

#ifndef STRING_FUNCTIONALITY_H
#define STRING_FUNCTIONALITY_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Data

#define KEY_MAX_LEN 255

// Public API (function protorypes)

unsigned char *ustrdup(const unsigned char *src);
int ustrcmp(const unsigned char* s1, const unsigned char* s2);
int is_key_valid(const unsigned char* key);

#endif
