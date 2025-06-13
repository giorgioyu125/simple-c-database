// Header
#include "string_functionality.h"

// Strings functionality

unsigned char** split_string_by_space(unsigned char* input_str, size_t* token_count) {
    if (input_str == NULL || token_count == NULL) {
        return NULL;
    }

    size_t capacity = 8; 
    size_t count = 0;

    unsigned char** tokens = malloc(capacity * sizeof(unsigned char*));
    if (tokens == NULL) {
        return NULL;
    }

    tokens[count++] = input_str;

    while (*input_str != '\0') {
        if (*input_str == ' ') {
            *input_str = '\0'; 
            if (*(input_str + 1) != '\0') {
                if (count >= capacity) {
                    capacity *= 2; 
                    unsigned char** new_tokens = realloc(tokens, capacity * sizeof(unsigned char*));
                    if (new_tokens == NULL) {
                        free(tokens); 
                        return NULL;
                    }
                    tokens = new_tokens;
                }
                tokens[count++] = input_str + 1;
            }
        }
        input_str++;
    }

    tokens[count] = NULL;
    *token_count = count;

    return tokens;
}

unsigned char* key_formatter(const char* input_key){
    if (input_key == NULL) {
        return NULL;
    }

    unsigned char* formatted_key = (unsigned char*)malloc(KEY_MAX_LEN);
    if (formatted_key == NULL) {
        return NULL;
    }

    strncpy((char*)formatted_key, input_key, KEY_MAX_LEN);

    size_t len = strlen(input_key);
    if (len < KEY_MAX_LEN) {
        memset(formatted_key + len, 0, KEY_MAX_LEN - len);
    }

    return formatted_key;
}


unsigned char* ustrdup(const unsigned char *src){
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

int is_key_valid(const unsigned char* key){
    if (key == NULL) {
        return 0;
    }

    if (key[0] == '\0') {
        return 0;
    }

    for (size_t i = 0; key[i] != '\0'; ++i) {

        if (!isalnum((unsigned char)key[i])) {
            return 0; 
        }
    }

    return 1;
}
