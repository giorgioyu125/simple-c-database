// Headers
#include "hashtable.h"    
#include "hashing_functionality.h"
#include "string_functionality.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>


// Lifecycle
hashtable_t* table_create(size_t initial_capacity){
    if (initial_capacity == 0){
        return NULL;
    }

    hashtable_t* new_hashtable = (hashtable_t*)malloc(sizeof(hashtable_t));
    if (new_hashtable == NULL){
        return NULL;
    }

    #if ENABLE_ONLY_POWER_2_SIZE
    if ((initial_capacity & (initial_capacity - 1)) != 0){
        initial_capacity = next_power_of_2(initial_capacity);
        fprintf(stderr, "[INFO] Requested capacity %zu is not a power of two. Adjusting to %zu for performance mode.\n",
                __atomic_load_n(&new_hashtable->elem_count, __ATOMIC_RELAXED),
                initial_capacity);
    }
    #endif

    new_hashtable->elem_count = 0;
    new_hashtable->buckets_count = initial_capacity;
    new_hashtable->lock_count = initial_capacity;

    new_hashtable->buckets = calloc(new_hashtable->buckets_count, sizeof(hashtable_bucket_t));
    if (!new_hashtable->buckets) {
        free(new_hashtable);
        return NULL;
    }
    
    new_hashtable->locks = malloc(new_hashtable->lock_count * sizeof(pthread_rwlock_t));
    if (!new_hashtable->locks) {
        free(new_hashtable->buckets);
        free(new_hashtable);
        return NULL;
    }

    for (size_t i = 0; i < new_hashtable->lock_count; ++i) {
        if (pthread_rwlock_init(&new_hashtable->locks[i], NULL) != 0) {
            for (size_t j = 0; j < i; ++j) {
                pthread_rwlock_destroy(&new_hashtable->locks[j]);
            }

            free(new_hashtable->locks);
            free(new_hashtable->buckets);
            free(new_hashtable);
            return NULL;
        }
    }

    return new_hashtable;
}

int table_destroy(hashtable_t* table, void (*value_destroyer)(void*)) {
    if (table == NULL) {
        return 0;
    }

    if (value_destroyer != NULL) {
        for (size_t i = 0; i < table->buckets_count; i++) {
            for (size_t j = 0; j < BUCKET_CAPACITY; j++) {
                if (table->buckets[i].in_use[j]) {
                    value_destroyer(table->buckets[i].values[j]);
                }
            }
        }
    }

    for (size_t i = 0; i < table->lock_count; i++) {
        if (pthread_rwlock_destroy(&table->locks[i]) != 0) {
            return -1;
        }
    }

    free(table->locks);
    free(table->buckets);
    free(table);

    return 0;
}

int table_clear(hashtable_t* table, void (*value_destroyer)(void*)) {
    if (table == NULL) {
        return -1; 
    }

    for (size_t i = 0; i < table->lock_count; i++) {
        if (pthread_rwlock_wrlock(&table->locks[i]) != 0) {
            for (size_t j = 0; j < i; j++) {
                pthread_rwlock_unlock(&table->locks[j]);
            }

            return -1; 
        }
    }

    for (size_t i = 0; i < table->buckets_count; i++) {
        for (size_t j = 0; j < BUCKET_CAPACITY; j++) {
            if (table->buckets[i].in_use[j]) {
                if (value_destroyer != NULL) {
                    value_destroyer(table->buckets[i].values[j]);
                }

                table->buckets[i].in_use[j] = 0;
                table->buckets[i].hashes[j] = 0; 

                table->buckets[i].values[j] = NULL; 
            }
        }
    }

    table->elem_count = 0;

    for (size_t i = 0; i < table->lock_count; i++) {
        pthread_rwlock_unlock(&table->locks[i]);
    }

    return 0; 
}

int table_resize(hashtable_t* table, size_t new_capacity) {
    if ((table == NULL) || (new_capacity == 0)){
        return -1;
    }

    if (new_capacity < table->elem_count){
        return -1;
    }

    if (new_capacity == table->buckets_count){
        return 0;
    }
    
    #if ENABLE_ONLY_POWER_2_SIZE
    if ((new_capacity & (new_capacity - 1)) != 0){
        new_capacity = next_power_of_2(new_capacity);
        fprintf(stderr, "[INFO] Requested capacity %zu is not a power of two. Adjusting to %zu for performance mode.\n",
                __atomic_load_n(&table->elem_count, __ATOMIC_RELAXED),
                new_capacity);
    }
    #endif

    for (size_t i = 0; i < table->lock_count; ++i){
        if (pthread_rwlock_wrlock(&table->locks[i]) != 0){
            fprintf(stderr, "[ERROR] table_resize: Failed to acquire lock %zu.\n", i);
            for (size_t j = 0; j < i; ++j) {
                pthread_rwlock_unlock(&table->locks[j]);
            }
            return -1;
        }
    }

    hashtable_bucket_t* new_buckets = calloc(new_capacity, sizeof(hashtable_bucket_t));
    if (new_buckets == NULL){
        fprintf(stderr, "[ERROR] table_resize: Failed to allocate new buckets.\n");
        for (size_t i = 0; i < table->lock_count; ++i) pthread_rwlock_unlock(&table->locks[i]);
        return -1;
    }

    pthread_rwlock_t* new_locks = malloc(new_capacity * sizeof(pthread_rwlock_t));
    if (new_locks == NULL){
        fprintf(stderr, "[ERROR] table_resize: Failed to allocate new locks.\n");
        free(new_buckets); 
        for (size_t i = 0; i < table->lock_count; ++i) pthread_rwlock_unlock(&table->locks[i]);
        return -1;
    }

    for (size_t i = 0; i < new_capacity; i++){
        if (pthread_rwlock_init(&new_locks[i], NULL) != 0){
            fprintf(stderr, "[ERROR] table_resize: Failed to initialize new lock.\n");
            for (size_t j = 0; j < i; j++) pthread_rwlock_destroy(&new_locks[j]);
            free(new_locks);
            free(new_buckets);
            for (size_t k = 0; k < table->lock_count; ++k) pthread_rwlock_unlock(&table->locks[k]);
            return -1;
        }
    }

    for (size_t i = 0; i < table->buckets_count; ++i){
        for (size_t j = 0; j < BUCKET_CAPACITY; ++j){
            if (table->buckets[i].in_use[j]){

                uint64_t hash = table->buckets[i].hashes[j];
                size_t new_bucket_index = hash % new_capacity;
                int inserted = 0;
                for (size_t k = 0; k < BUCKET_CAPACITY; ++k){
                    if (new_buckets[new_bucket_index].in_use[k] == 0){
                        new_buckets[new_bucket_index].in_use[k] = 1;
                        new_buckets[new_bucket_index].hashes[k] = hash;
                        new_buckets[new_bucket_index].values[k] = table->buckets[i].values[j];
                        memcpy(new_buckets[new_bucket_index].keys[k], table->buckets[i].keys[j], KEY_MAX_LEN);
                        inserted = 1;
                        break;
                    }
                }
                if (inserted == 0){
                    fprintf(stderr, "[FATAL] table_resize: Rehashing failed, bucket full.\n");

                    for(size_t k = 0; k < new_capacity; ++k) pthread_rwlock_destroy(&new_locks[k]);
                    free(new_locks);
                    free(new_buckets);
                    for (size_t k = 0; k < table->lock_count; ++k) pthread_rwlock_unlock(&table->locks[k]);
                    return -1;
                }
            }
        }
    }

    hashtable_bucket_t* old_buckets = table->buckets;
    pthread_rwlock_t* old_locks = table->locks;
    size_t old_lock_count = table->lock_count;

    table->buckets = new_buckets;
    table->buckets_count = new_capacity;
    table->locks = new_locks;
    table->lock_count = new_capacity;

    for (size_t i = 0; i < old_lock_count; ++i){
        pthread_rwlock_unlock(&old_locks[i]);
    }

    for (size_t i = 0; i < old_lock_count; ++i){
        pthread_rwlock_destroy(&old_locks[i]);
    }
    free(old_locks);
    free(old_buckets);

    return 0; 
}

// Core Ops

int table_set(hashtable_t* table, const unsigned char* key, void* value, void (*value_destroyer)(void*)){
    if ((table == NULL) || (key == NULL)){
        return -1;
    }

    unsigned long hash_full = hash(key);

    #if defined(ENABLE_ONLY_POWER_2_SIZE) && (ENABLE_ONLY_POWER_2_SIZE == 1)
    size_t bucket_index = hash_full & (table->buckets_count - 1);
    #else
    size_t bucket_index = hash_full % table->buckets_count;
    #endif

    if (pthread_rwlock_wrlock(&table->locks[bucket_index]) != 0){
        return -1;
    }

    hashtable_bucket_t* bucket = &table->buckets[bucket_index];
    int first_empty_slot = -1; 

    for (int i = 0; i < BUCKET_CAPACITY; i++){
        if ((bucket->in_use[i]) && 
            ((bucket->hashes[i]) == hash_full) && 
            (ustrcmp(bucket->keys[i], key) == 0)){
            if ((value_destroyer != NULL) && (bucket->values[i] != NULL)) {
                value_destroyer(bucket->values[i]);
            }

            bucket->values[i] = value;

            pthread_rwlock_unlock(&table->locks[bucket_index]);
            return 0; 
        } else if (!bucket->in_use[i]){
            if (first_empty_slot == -1) {
                first_empty_slot = i;
            }
        }
    }

    if (first_empty_slot != -1) {
        int i = first_empty_slot;

        bucket->in_use[i] = 1;
        bucket->hashes[i] = hash_full;
        bucket->values[i] = value;
        memcpy(bucket->keys[i], key, KEY_MAX_LEN); 

        __atomic_add_fetch(&table->elem_count, 1, __ATOMIC_RELAXED);

        pthread_rwlock_unlock(&table->locks[bucket_index]);
        return 0;
    }

    pthread_rwlock_unlock(&table->locks[bucket_index]);
    return -2; 
}

void* table_get(hashtable_t* table, const unsigned char* key, size_t (*value_sizer)(const void*)) {
    if (table == NULL || key == NULL || value_sizer == NULL) {
        return NULL;
    }

    uint64_t hash_full = hash(key);
    #if defined(ENABLE_ONLY_POWER_2_SIZE) && (ENABLE_ONLY_POWER_2_SIZE == 1)
    size_t bucket_index = hash_full & (table->buckets_count - 1);
    #else
    size_t bucket_index = hash_full % table->buckets_count;
    #endif

    hashtable_bucket_t* bucket = &table->buckets[bucket_index];

    if (pthread_rwlock_rdlock(&table->locks[bucket_index]) != 0) {
        return NULL;
    }

    void* internal_value = NULL;

    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if (bucket->in_use[i] &&
            bucket->hashes[i] == hash_full &&
            ustrcmp(bucket->keys[i], key) == 0) {

            internal_value = bucket->values[i];
            break;
        }
    }

    if (internal_value == NULL) {
        pthread_rwlock_unlock(&table->locks[bucket_index]);
        return NULL;
    }

    size_t total_size = value_sizer(internal_value);

    if (total_size == 0) {
        pthread_rwlock_unlock(&table->locks[bucket_index]);
        return NULL; 
    }

    void* value_copy = malloc(total_size);

    if (value_copy != NULL) {
        memcpy(value_copy, internal_value, total_size);
    }

    pthread_rwlock_unlock(&table->locks[bucket_index]);

    return value_copy;
}

int table_delete(hashtable_t* table, const unsigned char* key, void (*value_destroyer)(void*)) {
    if ((table == NULL) || (key == NULL)) {
        return -3; 
    }

    uint64_t hash_full = hash(key);
    #if defined(ENABLE_ONLY_POWER_2_SIZE) && (ENABLE_ONLY_POWER_2_SIZE == 1)
    size_t bucket_index = hash_full & (table->buckets_count - 1);
    #else
    size_t bucket_index = hash_full % table->buckets_count;
    #endif

    if (pthread_rwlock_wrlock(&table->locks[bucket_index]) != 0) {
        return -2; 
    }

    hashtable_bucket_t* bucket = &table->buckets[bucket_index];

    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if ((bucket->in_use[i]) &&
            (bucket->hashes[i] == hash_full) &&
            (ustrcmp(bucket->keys[i], key) == 0)) {

            bucket->in_use[i] = 0; 

            if ((value_destroyer != NULL) && (bucket->values[i] != NULL)) {
                value_destroyer(bucket->values[i]);
            }

            bucket->values[i] = NULL;
            bucket->hashes[i] = 0; 

            __atomic_sub_fetch(&table->elem_count, 1, __ATOMIC_RELAXED);

            pthread_rwlock_unlock(&table->locks[bucket_index]);
            return 0; 
        }
    }

    pthread_rwlock_unlock(&table->locks[bucket_index]);
    return -1; 
}

bool table_exist(hashtable_t* table, const unsigned char* key){
    if ((table == NULL) || (key == NULL)){
        return false;
    }

    uint64_t hash_full = hash(key);
    #if defined(ENABLE_ONLY_POWER_2_SIZE) && (ENABLE_ONLY_POWER_2_SIZE == 1)
    size_t bucket_index = hash_full & (table->buckets_count - 1);
    #else
    size_t bucket_index = hash_full % table->buckets_count;
    #endif

    if (pthread_rwlock_rdlock(&table->locks[bucket_index]) != 0){
        return false; 
    }

    hashtable_bucket_t* bucket = &table->buckets[bucket_index];
    bool found = false; 

    for (int i = 0; i < BUCKET_CAPACITY; i++){
        if (bucket->in_use[i] &&
            bucket->hashes[i] == hash_full &&
            ustrcmp(bucket->keys[i], key) == 0){

            found = true;
            break; 
        }
    }

    pthread_rwlock_unlock(&table->locks[bucket_index]);

    return found;
}

int table_add(hashtable_t* table, const unsigned char* key, void* value) {
    if ((table == NULL) || (key == NULL)) {
        return -1; 
    }

    uint64_t hash_full = hash(key);
    #if defined(ENABLE_ONLY_POWER_2_SIZE) && (ENABLE_ONLY_POWER_2_SIZE == 1)
    size_t bucket_index = hash_full & (table->buckets_count - 1);
    #else
    size_t bucket_index = hash_full % table->buckets_count;
    #endif

    if (pthread_rwlock_wrlock(&table->locks[bucket_index]) != 0) {
        return -1; 
    }

    hashtable_bucket_t* bucket = &table->buckets[bucket_index];
    int first_empty_slot = -1;

    for (int i = 0; i < BUCKET_CAPACITY; i++) {

        if (bucket->in_use[i] &&
            bucket->hashes[i] == hash_full &&
            ustrcmp(bucket->keys[i], key) == 0) {

            pthread_rwlock_unlock(&table->locks[bucket_index]);
            return -3; 
        }

        if (!bucket->in_use[i] && first_empty_slot == -1) {
            first_empty_slot = i;
        }
    }

    if (first_empty_slot != -1) {
        int i = first_empty_slot;

        bucket->in_use[i] = 1;
        bucket->hashes[i] = hash_full;
        memcpy(bucket->keys[i], key, KEY_MAX_LEN);
        bucket->values[i] = value;

        __atomic_add_fetch(&table->elem_count, 1, __ATOMIC_RELAXED);

        pthread_rwlock_unlock(&table->locks[bucket_index]);
        return 0; 
    }

    pthread_rwlock_unlock(&table->locks[bucket_index]);
    return -2; 
}

int table_replace(hashtable_t* table, const unsigned char* key, void* new_value, void (*value_destroyer)(void*)) {
    if ((table == NULL) || (key == NULL)) {
        return -1; 
    }

    uint64_t hash_full = hash(key);
    #if defined(ENABLE_ONLY_POWER_2_SIZE) && (ENABLE_ONLY_POWER_2_SIZE == 1)
    size_t bucket_index = hash_full & (table->buckets_count - 1);
    #else
    size_t bucket_index = hash_full % table->buckets_count;
    #endif

    if (pthread_rwlock_wrlock(&table->locks[bucket_index]) != 0) {
        return -1; 
    }

    hashtable_bucket_t* bucket = &table->buckets[bucket_index];

    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if (bucket->in_use[i] &&
            bucket->hashes[i] == hash_full &&
            ustrcmp(bucket->keys[i], key) == 0) {

            if (value_destroyer != NULL && bucket->values[i] != NULL) {
                value_destroyer(bucket->values[i]);
            }

            bucket->values[i] = new_value;

            pthread_rwlock_unlock(&table->locks[bucket_index]);
            return 0; 
        }
    }

    pthread_rwlock_unlock(&table->locks[bucket_index]);
    return -2; 
}


// Monitoring

double table_load_factor(hashtable_t* table) {
    if (table == NULL) {
        return 0.0;
    }

    size_t capacity = table->buckets_count * BUCKET_CAPACITY;
    if (capacity == 0) {
        return 0.0;
    }

    size_t count = __atomic_load_n(&table->elem_count, __ATOMIC_RELAXED);

    return (double)count / (double)capacity;
}

  // Count


size_t table_memory_usage(hashtable_t* table, size_t (*value_sizer)(const void* value)){
    if (table == NULL){
        return 0;
    }

    size_t total_size = 0;
    total_size += sizeof(hashtable_t);
    total_size += table->buckets_count * sizeof(hashtable_bucket_t);
    total_size += table->lock_count * sizeof(pthread_rwlock_t);

    if (value_sizer == NULL){
        return total_size; 
    }

    for (size_t i = 0; i < table->buckets_count; i++){
        if (pthread_rwlock_rdlock(&table->locks[i]) != 0){
            fprintf(stderr, "[ERROR] table_memory_usage: Failed to acquire read lock on bucket %zu.\n", i);

            for (size_t k = 0; k < i; k++){
                pthread_rwlock_unlock(&table->locks[k]);
            }

            return (size_t)-1; 
        }
    }

    for (size_t i = 0; i < table->buckets_count; i++){
        for (int j = 0; j < BUCKET_CAPACITY; j++) {
            if (table->buckets[i].in_use[j] && 
                table->buckets[i].values[j] != NULL) {
                total_size += value_sizer(table->buckets[i].values[j]);
            }
        }
    }
    
    for (size_t i = 0; i < table->buckets_count; i++){
        pthread_rwlock_unlock(&table->locks[i]);
    }

    return total_size;
}

    
size_t table_capacity(hashtable_t* table) {
    if (table == NULL) {
        return 0; 
    }

    return table->buckets_count * BUCKET_CAPACITY;
}

double table_occupied_bucket_counter(hashtable_t* table) {
    if (table == NULL || table->buckets_count == 0) {
        return 0.0;
    }

    size_t occupied_buckets = 0;
    for (size_t i = 0; i < table->buckets_count; i++) {
        if (pthread_rwlock_rdlock(&table->locks[i]) == 0) {
            for (int j = 0; j < BUCKET_CAPACITY; j++) {
                if (table->buckets[i].in_use[j]) {
                    occupied_buckets++;
                    break;
                }
            }
            pthread_rwlock_unlock(&table->locks[i]);
        }
    }

    return (double)occupied_buckets / (double)table->buckets_count;
}

size_t table_total_elem(hashtable_t* table){
    return __atomic_load_n(&table->elem_count, __ATOMIC_RELAXED);
}
