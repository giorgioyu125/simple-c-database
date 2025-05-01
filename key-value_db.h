#pragma once


// Includes

#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Table Parameters

#define TABLE_SIZE 4096

// Error Outputs

#define DB_SUCCESS 0
#define DB_FAILURE -1       
#define DB_KEY_NOT_FOUND -2
#define DB_MEM_ERROR -3     
#define DB_KEY_EXISTS -4   

// Debugging (Set to 1 to enable this mode)
#define DEBUG_MODE 1


// Data

typedef struct Node{
    unsigned char *key;
    void *value;
    size_t value_size;
    struct Node *next;
} Node;

Node** hashTable = NULL;


// Other functionality

int ustrcmp(const unsigned char* s1, const unsigned char* s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    // Compare the differing characters or the null terminators
    return (int)(*s1) - (int)(*s2);
}


// Hashing Function

unsigned long hash(const unsigned char *str) {
    unsigned long hash = 5381;
    unsigned long c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash;
}


// DataBase functionality

int db_delete(const unsigned char* key) {
    if (key == NULL) {
        fprintf(stderr, "[ERROR] db_delete: Key cannot be null.\n");
        return DB_FAILURE; 
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] db_get: Database is not initialized.\n");
        return NULL;
    }

    const unsigned long index = (hash(key)) & (TABLE_SIZE - 1);

    Node* current = hashTable[index];
    Node* previous = NULL;

    while (current != NULL) {

        if (ustrcmp(current->key, key) == 0) {
            if (previous == NULL) {

                hashTable[index] = current->next;
            } else {

                previous->next = current->next;
            }

            free(current->key);   
            free(current->value); 
            free(current);        

            return DB_SUCCESS; 
        }

        previous = current;
        current = current->next;
    }

    fprintf(stderr, "[INFO] db_delete: Key '%s' not found.\n", key); 
    return DB_KEY_NOT_FOUND;
}

void* db_get(const unsigned char* key, size_t* out_value_size) {
    if (out_value_size == NULL) {
        fprintf(stderr, "[ERROR] db_get: Out parameter cannot be Null.\n");
        return NULL;
    }

    *out_value_size = 0;

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] db_get: Database is not initialized.\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "[ERROR] db_get: Provided key is Null.\n");
        return NULL;
    }

    const unsigned long index = (hash(key)) & (TABLE_SIZE - 1);

    Node* current = hashTable[index];
    
    while (current != NULL) {
        if (ustrcmp(current->key, key) == 0){
            
            *out_value_size = current->value_size;

            void *value = current->value; 

            #if DEBUG_MODE
                printf("[DEBUG] db_get: Key found -> %s, Value size -> %lu, Value -> %p\n", key, *out_value_size, value);
            #endif

            return value;
        }
        current = current->next;
    }

    fprintf(stderr, "[ERROR] db_get: Key not found.\n");
    return NULL; 
}

bool db_exist(const unsigned char* key) {
    if (key == NULL){
        fprintf(stderr, "[ERROR] db_exist: The Key cannot be Null.\n");
        return DB_FAILURE;
    }

    if (hashTable == NULL){
        fprintf(stderr, "[ERROR] db_exist: The database is not initialized.\n");
        return DB_FAILURE;
    }

    int index = (hash(key) & (TABLE_SIZE - 1));
    
    Node* current = hashTable[index];
    int counter = 1;
    while (current != NULL){
        #if DEBUG_MODE
            printf("[DEBUG] db_exist: Position in the list of index %d -> %d", index, counter++);
        #endif

        if (ustrcmp(current->key, key) == 0){
            return true;
        }

        current = current->next;
    }

    return false;
}

int db_set(const unsigned char* key, const void* value_ptr, size_t value_size) {

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] db_set: Database not initialized.\n");
        return DB_FAILURE;
    }

    if (key == NULL) {
        fprintf(stderr, "[ERROR] db_set: Key cannot be NULL.\n");
        return DB_FAILURE;
    }
    
    if (value_size <= 0) {
        fprintf(stderr, "[ERROR] db_set: value_size needs to be greater than 0.\n");
        return DB_FAILURE;
    }
    
    if (value_ptr == NULL){
        fprintf(stderr, "[ERROR] db_set: value_ptr cannot be NULL in any case\n");
        return DB_FAILURE;
    }

    const unsigned long index = (hash(key)) & (TABLE_SIZE - 1);
    #if DEBUG_MODE
        printf("[DEBUG] db_set: key = %s, value = %p, value_size = %zu, accessing to: hashTable[%lu]", key, value_ptr, value_size, index);
    #endif
    
    Node* current = hashTable[index]; // This is the head of the linked list where we will store the values with the same hashTable
    while (current != NULL) {

        if (ustrcmp(current->key, key) == 0){

            #if DEBUG_MODE
                printf("[DEBUG] db_set: Key '%s' found in the index %lu. Updating the value.\n", key, index);
            #endif
            
            void *new_value = malloc(value_size);

            if (new_value == NULL){
                fprintf(stderr, "[ERROR] db_set : Allocation failed.");
                return DB_MEM_ERROR;
            }

            memcpy(new_value, value_ptr, value_size);

            free(current->value);

            current->value = new_value;
            current->value_size = value_size;

            return DB_SUCCESS;
        }
        current = current->next;
    }

    #if DEBUG_MODE
        printf("[DEBUG] db_set: Key '%s' not found at index %lu. Inserting new node at head.\n", key, index);
    #endif


    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "[ERROR] db_set (insert): Failed to allocate memory for Node structure");
        return DB_MEM_ERROR;
    }

    newNode->key = (unsigned char*)strdup((char*)key);
    if (newNode->key == NULL) {
        fprintf(stderr, "[ERROR] db_set (insert): Failed to duplicate key string");
        free(newNode); // IMPORTANT: Clean up the partially allocated node
        return DB_MEM_ERROR;
    }
    

    newNode->value = NULL; // Initialize to NULL, handles the value_size == 0 case
    if (value_size > 0) {
        newNode->value = malloc(value_size);
        if (newNode->value == NULL) {
            fprintf(stderr, "[ERROR] db_set (insert): Failed to allocate memory for value data");

            free(newNode->key);
            free(newNode);
            return DB_MEM_ERROR;
        }
        memcpy(newNode->value, value_ptr, value_size);
    }
    newNode->value_size = value_size;

    newNode->next = hashTable[index];
    hashTable[index] = newNode;

    #if DEBUG_MODE
        printf("[DEBUG] db_set: Successfully inserted key '%s' at index %lu.\n", key, index);
    #endif

    return DB_SUCCESS; 
}


int destroy_db(void){
    if (hashTable == NULL) return DB_FAILURE;
    #if DEBUG_MODE
        printf("[DEBUG]: destroy_db: Starting database destruction...\n");
    #endif

    for (int i = 0; i < TABLE_SIZE; i++){
        Node* current = hashTable[i];
        
        while (current != NULL){
            Node* next_node = current->next;

            #if DEBUG_MODE
                printf("[DEBUG] destroy_db: destroy_db: Starting node index %d destruction...\n", i);
            #endif

            free(current->key);
            free(current->value);
            free(current);
            
            current = next_node;
        }
    }

    free(hashTable);
    hashTable = NULL;

    #if DEBUG_MODE
        printf("[DEBUG] destroy_db: Database destroyed successfully.\n");
    #endif

    return DB_SUCCESS;
}


int init_db(void){
    if (hashTable != NULL){
        #if DEBUG_MODE
            fprintf(stderr, "[ERROR] init_db: Database already initialized.\n");
        #endif

        return DB_FAILURE;
    }

    hashTable = (Node**)calloc(TABLE_SIZE, sizeof(Node*));
    
    if (hashTable == NULL){
        #if DEBUG_MODE
            fprintf(stderr, "[ERROR] init_db: Allocation failed.\n");
        #endif
        
        return DB_MEM_ERROR;
    }

    #if DEBUG_MODE
        printf("[DEBUG] init_db: Database initialized successfully with table size %d.\n", TABLE_SIZE);
    #endif

    return DB_SUCCESS;
}
