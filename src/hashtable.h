#ifndef HASHTABLE_H
#define HASHTABLE_H

// INCLUDES

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include "string_functionality.h"

// MACRO

#define LIKELY(x)                 __builtin_expect(!!(x), 1)
#define UNLIKELY(x)               __builtin_expect(!!(x), 0)

#define BUCKET_CAPACITY           8

#define ENABLE_ONLY_POWER_2_SIZE  1

// DATA

typedef struct __attribute__((aligned(64))) hashtable_bucket_t {
    uint8_t in_use[BUCKET_CAPACITY];
    uint64_t hashes[BUCKET_CAPACITY];

    unsigned char keys[BUCKET_CAPACITY][KEY_MAX_LEN];
    void* values[BUCKET_CAPACITY];
} hashtable_bucket_t;


typedef struct hashtable_t{
    hashtable_bucket_t* buckets;
    size_t buckets_count;
    _Atomic(size_t) elem_count;

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
    void* table_get(hashtable_t* table, const unsigned char* key, size_t (*value_sizer)(const void*));
    int table_delete(hashtable_t* table, const unsigned char* key, void (*value_destroyer)(void*));
    bool table_exist(hashtable_t* table, const unsigned char* key);
    int table_add(hashtable_t* table, const unsigned char* key, void* value);
    int table_replace(hashtable_t* table, const unsigned char* key, void* new_value, void (*value_destroyer)(void*));

    // Monitoring
    size_t table_memory_usage(hashtable_t* table, size_t (*value_sizer)(const void* value));
    size_t table_capacity(hashtable_t* table);
    double table_load_factor(hashtable_t* table);
    double table_occupied_bucket_counter(hashtable_t* table);
    size_t table_total_elem(hashtable_t* table);

#endif
