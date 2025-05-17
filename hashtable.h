// Includes

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "string_functionality.h"
#include "hashing_functionality.h"

// Defines

    // Table Parameters

    #define TABLE_SIZE 4096

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

typedef struct Node{
    unsigned char *key;
    void *value;
    size_t value_size;
    struct Node *next;
} Node;

// Public API (function prototypes)

int db_delete(const unsigned char* key);
void* db_get(const unsigned char* key, size_t* out_value_size);
bool db_exist(const unsigned char* key);
int db_set(const unsigned char* key, const void* value_ptr, size_t value_size);
int destroy_db(void);
int init_db(void);
int db_save(const char* filename);
int db_load(const char* filename);

#endif
