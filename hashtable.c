#include "hashtable.h"    
#include <stdlib.h>


// Lifecycle
hashtable_t* table_create(size_t initial_capacity){
    if (initial_capacity == 0){
        return NULL;
    }

    hashtable_t* new_hashtable = (hashtable_t*)malloc(sizeof(hashtable_t));
    if (new_hashtable == NULL){
        return NULL;
    }

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
        // Table needs initialization
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
            if (!table->buckets[i].in_use[j]) {
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

int table_resize(hashtable_t* table, size_t new_capacity){

}

// Core Ops
int table_set(hashtable_t* table, char* key, void* value){

}

void* table_get(hashtable_t* table, char* key){

}

int table_delete(hashtable_t* table, char* key){

}

bool table_exist(hashtable_t* table, char* key){

}

int table_add(hashtable_t* table, char* key, void* value){

}

int table_replace(hashtable_t* table, char* key, void* new_value){

}


// Monitoring
size_t table_count(hashtable_t* table){

}
    
size_t table_capacity(hashtable_t* table){

}

int table_loadfactor(hashtable_t* table){

}


// Iterator
int table_foreach(hashtable_t* table, void* callback[], char* data) {

}
