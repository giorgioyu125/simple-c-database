

// Includes
#ifndef HASHSET_H
#define HASHSET_H

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uv.h>
#include "string_functionality.h"
#include "hashing_functionality.h"

// Defines

    // Table Parameters

    #define MATHEMATICAL_SET 0
    #define LOAD_OVERLAP 1

    // Error Outputs

    #define DB_SUCCESS 0
    #define DB_FAILURE -1       
    #define DB_KEY_NOT_FOUND -2
    #define DB_MEM_ERROR -3     
    #define DB_KEY_EXISTS -4   
    #define DB_FILE_CORRUPT -5
    #define DB_FILE_ERROR -6

    // Debugging (Set to 1 to enable this mode)

    #define DEBUG_MODE 1

// Data

typedef struct Cell{
    unsigned char *key;
    struct Cell* next;
} Cell;

typedef struct HashSet {
    Cell** buckets;
    size_t capacity;
    size_t count;    
    uv_mutex_t mutex;
} HashSet;

// Public API (Functions property)

int set_init(const unsigned int capacity);
int set_destroy(void);
int set_add(unsigned char* key);
int set_delete(unsigned char* key);
int set_save(const char* filename);
int set_load(const char* filename);
bool set_exist(const unsigned char* key, int* out_error_value);

#endif
