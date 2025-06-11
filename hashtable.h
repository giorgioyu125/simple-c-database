// INCLUDES

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "string_functionality.h"
#include "hashing_functionality.h"

// MACRO

#define BUCKET_CAPACITY 8

// DATA

typedef struct hashtable_bucket_t{
    uint8_t in_use[BUCKET_CAPACITY];
    uint64_t hashes[BUCKET_CAPACITY];

    unsigned char keys[BUCKET_CAPACITY][KEY_MAX_LEN];
    void* values[BUCKET_CAPACITY];
} hashtable_bucket_t;


typedef struct hashtable_t{
    hashtable_bucket_t* buckets;
    size_t buckets_count;
    size_t elem_count;

    pthread_rwlock_t* locks;
    size_t lock_count;
} hashtable_t;

// API
    
    // Lifecycle
    hashtable_t* table_create(size_t initial_capacity);
    int table_destroy(hashtable_t* table, void (*value_destroyer)(void*));
    int table_clear(hashtable_t* table, void (*value_destroyer)(void*));
    int table_resize(hashtable_t* table, size_t new_capacity);

    // Core Ops
    int table_set(hashtable_t* table, const unsigned char* key, void* value, void (*value_destroyer)(void*));
    void* table_get(hashtable_t* table, const unsigned char* key);
    int table_delete(hashtable_t* table, const unsigned char* key, void (*value_destroyer)(void*));
    bool table_exist(hashtable_t* table, const unsigned char* key);
    int table_add(hashtable_t* table, const unsigned char* key, void* value);
    int table_replace(hashtable_t* table, const unsigned char* key, void* new_value, void (*value_destroyer)(void*));

    // Monitoring
    size_t table_count(hashtable_t* table);
    size_t table_capacity(hashtable_t* table);
    int table_loadfactor(hashtable_t* table);

    
