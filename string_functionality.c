// Header
#include "string_functionality.h"

// Strings functionality

unsigned char* key_formatter(const char* input_key) {
    if (input_key == NULL) {
        return NULL;
    }

    unsigned char* formatted_key = (unsigned char*)malloc(KEY_MAX_LEN);
    if (formatted_key == NULL) {
        return NULL; // Errore di allocazione
    }

    strncpy((char*)formatted_key, input_key, KEY_MAX_LEN);

    size_t len = strlen(input_key);
    if (len < KEY_MAX_LEN) {
        memset(formatted_key + len, 0, KEY_MAX_LEN - len);
    }

    return formatted_key;
}


unsigned char *ustrdup(const unsigned char *src){
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
