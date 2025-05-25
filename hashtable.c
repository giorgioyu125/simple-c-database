// Includes

#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "string_functionality.h"
#include "hashing_functionality.h"
#include "hashtable.h"


// Hashtable functionality

int table_init(size_t db_size, Node** hashTable) {
    if (hashTable != NULL){
        #if DEBUG_MODE
            fprintf(stderr, "[ERROR] table_init: Database already initialized.\n");
        #endif

        return DB_FAILURE;
    }

    hashTable = (Node**)calloc(db_size, sizeof(Node*));
    
    if (hashTable == NULL){
        #if DEBUG_MODE
            fprintf(stderr, "[ERROR] table_init: Allocation failed.\n");
        #endif
        
        return DB_MEM_ERROR;
    }

    #if DEBUG_MODE
        printf("[DEBUG] table_init: Hashtable initialized successfully with table size %zu.\n", db_size);
    #endif

    return DB_SUCCESS;
}

int table_delete(const unsigned char* key, Node** hashTable) {
    if (key == NULL) {
        fprintf(stderr, "[ERROR] table_delete: Key cannot be null.\n");
        return DB_FAILURE; 
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] table_delete: Database is not initialized.\n");
        return DB_FAILURE;
}

    const unsigned long index = hash(key) % TABLE_SIZE;

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

    fprintf(stderr, "[ERROR] table_delete: Key '%s' not found.\n", key); 
    return DB_KEY_NOT_FOUND;
}

void* db_get(const unsigned char* key, size_t* out_value_size, Node** hashTable) {
    if (out_value_size == NULL) {
        fprintf(stderr, "[ERROR] table_get: Out parameter cannot be Null.\n");
        return NULL;
    }

    *out_value_size = 0;

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] table_get: Database is not initialized.\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "[ERROR] table_get: Provided key is Null.\n");
        return NULL;
    }

    const unsigned long index = hash(key) % TABLE_SIZE;

    Node* current = hashTable[index];
    
    while (current != NULL) {
        if (ustrcmp(current->key, key) == 0){
            
            *out_value_size = current->value_size;

            void *value = current->value; 

            #if DEBUG_MODE
                printf("[DEBUG] table_get: Key found -> %s, Value size -> %lu, Value -> %p\n", key, *out_value_size, value);
            #endif

            return value;
        }
        current = current->next;
    }

    fprintf(stderr, "[ERROR] table_get: Key not found.\n");
    return NULL; 
}

bool table_exist(const unsigned char* key, Node** hashTable) {
    if (key == NULL){
        fprintf(stderr, "[ERROR] table_exist: The Key cannot be Null.\n");

        return false; 
    }

    if (hashTable == NULL){
        fprintf(stderr, "[ERROR] table_exist: The database is not initialized.\n");

        return false; 
    }

    const unsigned long index = hash(key) % TABLE_SIZE;

    Node* current = hashTable[index];
    #if DEBUG_MODE
        int counter = 1; 
        printf("[DEBUG] table_exist: Checking index %lu for key '%s'.\n", index, key);
    #endif

    while (current != NULL){
        #if DEBUG_MODE
            printf("[DEBUG] table_exist: Comparing with node %d at index %lu (Key: '%s').\n", counter++, index, current->key);
        #endif

        if (ustrcmp(current->key, key) == 0){
             #if DEBUG_MODE
                 printf("[DEBUG] table_exist: Key '%s' found.\n", key);
             #endif
            return true; 
        }

    current = current->next;
    }

    #if DEBUG_MODE
         printf("[DEBUG] table_exist: Key '%s' not found after checking index %lu.\n", key, index);
    #endif
    return false; 
}

int table_set(const unsigned char* key, const void* value_ptr, size_t value_size, Node** hashTable) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] table_set: Database not initialized.\n");
        return DB_FAILURE;
    }

    if (key == NULL) {
        fprintf(stderr, "[ERROR] table_set: Key cannot be NULL.\n");
        return DB_FAILURE;
    }
    
    if (value_size <= 0) {
        fprintf(stderr, "[ERROR] table_set: value_size needs to be greater than 0.\n");
        return DB_FAILURE;
    }
    
    if (value_ptr == NULL) {
        fprintf(stderr, "[ERROR] table_set: value_ptr cannot be NULL in any case\n");
        return DB_FAILURE;
    }

    const unsigned long index = (hash(key)) % TABLE_SIZE;
    #if DEBUG_MODE
        printf("[DEBUG] table_set: key = %s, value = %p, value_size = %zu, accessing to: hashTable[%lu]", key, value_ptr, value_size, index);
    #endif
    
    Node* current = hashTable[index];
    while (current != NULL) {

        if (ustrcmp(current->key, key) == 0) {

            #if DEBUG_MODE
                printf("[DEBUG] table_set: Key '%s' found in the index %lu. Updating the value.\n", key, index);
            #endif
            
            void *new_value = malloc(value_size);

            if (new_value == NULL) {
                fprintf(stderr, "[ERROR] table_set : Allocation failed.");
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
        printf("[DEBUG] table_set: Key '%s' not found at index %lu. Inserting new node at head.\n", key, index);
    #endif


    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "[ERROR] table_set (insert): Failed to allocate memory for Node structure");
        return DB_MEM_ERROR;
    }

    newNode->key = ustrdup(key);
    if (newNode->key == NULL) {
        fprintf(stderr, "[ERROR] table_set (insert): Failed to duplicate key string");
        free(newNode); 
        return DB_MEM_ERROR;
    }
    

    newNode->value = NULL; 
    if (value_size > 0) {
        newNode->value = malloc(value_size);
        if (newNode->value == NULL) {
            fprintf(stderr, "[ERROR] table_set (insert): Failed to allocate memory for value data");

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
        printf("[DEBUG] table_set: Successfully inserted key '%s' at index %lu.\n", key, index);
    #endif

    return DB_SUCCESS; 
}


int table_destroy(Node** hashTable){
    if (hashTable == NULL) return DB_FAILURE;
    #if DEBUG_MODE
        printf("[DEBUG] table_destroy: Starting database destruction...\n");
    #endif

    for (int i = 0; i < TABLE_SIZE; i++){
        Node* current = hashTable[i];
        
        while (current != NULL){
            Node* next_node = current->next;

            #if DEBUG_MODE
                printf("[DEBUG] table_destroy: Starting node index %d destruction...\n", i);
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
        printf("[DEBUG] table_destroy: Database destroyed successfully.\n");
    #endif

    return DB_SUCCESS;
}


int table_save(const char* filename, Node** hashTable) {
#if DEBUG_MODE
    printf("[DEBUG] table_save: Attempting to save database to '%s'.\n", filename);
#endif

    if (hashTable == NULL) {

        fprintf(stderr, "[ERROR] table_save: HashTable is not initialized.\n");
        return DB_FAILURE;
    }

    char temp_filename[FILENAME_MAX];

    int len = snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);
    if (len < 0 || (size_t)len >= sizeof(temp_filename)) {
         fprintf(stderr, "[ERROR] table_save: Failed to create temporary filename (too long?).\n");
         return DB_FAILURE;
    }
#if DEBUG_MODE
    printf("[DEBUG] table_save: Using temporary file '%s'.\n", temp_filename);
#endif

    FILE *fp = fopen(temp_filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] table_save: Error creating temporary file '%s'.\n", temp_filename);
        return DB_FAILURE;
    }
#if DEBUG_MODE
    printf("[DEBUG] table_save: Temporary file '%s' opened successfully.\n", temp_filename);
#endif

#if DEBUG_MODE
    printf("[DEBUG] table_save: Starting iteration through hash table (size %d).\n", TABLE_SIZE);
    unsigned long nodes_written = 0; 
#endif

    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = hashTable[i];
#if DEBUG_MODE 
        if (current != NULL) {
            printf("[DEBUG] table_save: Processing bucket %d.\n", i);
        }
#endif

        while (current != NULL) {
            size_t klen = strlen((char*)current->key) + 1;
            size_t value_len = current->value_size;
            size_t items_written;

#if DEBUG_MODE
            printf("[DEBUG] table_save: Writing node - Key: '%s' (klen=%zu), Value Size: %zu\n", current->key, klen, value_len);
#endif

            items_written = fwrite(&klen, sizeof(size_t), 1, fp);
            if (items_written != 1) {goto write_error;}

            items_written = fwrite(current->key, 1, klen, fp);
            if (items_written != klen) {goto write_error;}

            items_written = fwrite(&value_len, sizeof(size_t), 1, fp);
            if (items_written != 1) {goto write_error;}

            if (value_len > 0) {
                 items_written = fwrite(current->value, 1, value_len, fp);
                 if (items_written != value_len) {goto write_error;}
            }

#if DEBUG_MODE
            nodes_written++;
#endif
            current = current->next;
        }
    }

#if DEBUG_MODE
    printf("[DEBUG] table_save: Finished table iteration. %lu nodes written to temporary file.\n", nodes_written);
#endif

    if (fclose(fp) == EOF) {
        fp = NULL;
        fprintf(stderr, "[ERROR] table_save: Failed to close temporary file '%s' after writing.\n", temp_filename);

        remove(temp_filename);
        return DB_FAILURE;
    }
    fp = NULL; 
#if DEBUG_MODE
    printf("[DEBUG] table_save: Temporary file '%s' closed successfully.\n", temp_filename);
#endif

    if (rename(temp_filename, filename) != 0) {
        fprintf(stderr, "[ERROR] table_save: Failed to rename temporary file '%s' to '%s'.\n", temp_filename, filename);
        remove(temp_filename); 
        return DB_FAILURE;
    }
#if DEBUG_MODE
    printf("[DEBUG] table_save: Successfully renamed '%s' to '%s'. Save complete.\n", temp_filename, filename);
#endif

    return DB_SUCCESS;

write_error:
#if DEBUG_MODE
    printf("[DEBUG] table_save: Entering write_error handler. Error occurred writing to '%s'.\n", temp_filename);
#endif

    fprintf(stderr, "[ERROR] table_save: Error writing to temporary file '%s'.\n", temp_filename);
    if (fp != NULL) {
        fclose(fp); 
    }

    remove(temp_filename); 
    return DB_FAILURE;
}

int table_load(const char* filename, size_t db_size, Node** hashTable) {
    FILE *file = NULL;
    unsigned char* temp_key_buffer = NULL;
    void* temp_value_buffer = NULL;
    size_t klen = 0, vlen = 0;
    size_t items_read = 0;
    int db_set_result = DB_SUCCESS;
    int overall_status = DB_SUCCESS;
    long record_count = 0;

    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] table_load: Attempting to load database from '%s'\n", filename);
    #endif

    if (filename == NULL) {
        fprintf(stderr, "[ERROR] table_load: Filename cannot be NULL.\n");
        return DB_FAILURE;
    }

    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] table_load: Clearing existing database (if any)...\n");
    #endif

    table_destroy(hashTable); 

    if (table_init(db_size, hashTable) != DB_SUCCESS) {
         fprintf(stderr, "[ERROR] table_load: Failed to re-initialize database after clearing.\n");
         return DB_MEM_ERROR;
    }
    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] table_load: Database initialized successfully.\n");
    #endif

    file = fopen(filename, "rb");
    if (file == NULL) {

        #if DEBUG_MODE
            fprintf(stderr, "[DEBUG] table_load: File '%s' not found or cannot be opened.\n", filename);
        #endif

        fprintf(stderr, "[ERROR] table_load: Failed to open file '%s'\n", filename);
        return DB_FILE_ERROR; 
    }

    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] table_load: File '%s' opened successfully.\n", filename);
    #endif

    temp_key_buffer = NULL;
    temp_value_buffer = NULL;

    while (true) { 

        items_read = fread(&klen, sizeof(size_t), 1, file);
        if (items_read != 1) {
            if (feof(file)) {
                #if DEBUG_MODE
                    fprintf(stderr, "[DEBUG] table_load: Reached end of file cleanly after %ld records.\n", record_count);
                #endif
                overall_status = DB_SUCCESS; 
            } else {

                fprintf(stderr, "[ERROR] table_load: Failed to read key length from file (Record %ld): Read error or premature EOF.\n", record_count + 1);
                overall_status = DB_FILE_CORRUPT; 
            }
            break; 
        }

        #if DEBUG_MODE

        #endif

        if (klen == 0) {
             fprintf(stderr, "[ERROR] table_load: Read invalid key length (0) (Record %ld).\n", record_count + 1);
             overall_status = DB_FILE_CORRUPT;
             break; 
        }

        free(temp_key_buffer); temp_key_buffer = NULL;
        free(temp_value_buffer); temp_value_buffer = NULL;

        temp_key_buffer = (unsigned char*)malloc(klen);
        if (temp_key_buffer == NULL) {
            fprintf(stderr, "[ERROR] table_load: Failed to allocate %zu bytes for key buffer (Record %ld).\n", klen, record_count + 1);
            overall_status = DB_MEM_ERROR;
            break; 
        }

        items_read = fread(temp_key_buffer, 1, klen, file);
        if (items_read != klen) {
            fprintf(stderr, "[ERROR] table_load: Failed to read %zu key bytes (read %zu) (Record %ld).\n", klen, items_read, record_count + 1);
            overall_status = DB_FILE_CORRUPT; 

            free(temp_key_buffer); temp_key_buffer = NULL;
            break; 
        }

        #if DEBUG_MODE
            fprintf(stderr, "[DEBUG] table_load: Record %ld: Read key (klen=%zu): '%.*s...'\n", record_count + 1, klen, (int)(klen > 50 ? 50 : klen-1), temp_key_buffer);
        #endif

        items_read = fread(&vlen, sizeof(size_t), 1, file);
            if (items_read != 1) {
                fprintf(stderr, "[ERROR] table_load: Failed to read value length from file (Record %ld)\n", record_count + 1);
                overall_status = DB_FILE_CORRUPT; 

                free(temp_key_buffer); temp_key_buffer = NULL;
                break; 
        }
        #if DEBUG_MODE
            fprintf(stderr, "[DEBUG] table_load: Record %ld: Read vlen = %zu\n", record_count + 1, vlen);
        #endif

        temp_value_buffer = NULL; 
        if (vlen > 0) {
            temp_value_buffer = malloc(vlen);
            if (temp_value_buffer == NULL) {
                 fprintf(stderr, "[ERROR] table_load: Failed to allocate %zu bytes for value buffer (Record %ld).\n", vlen, record_count + 1);
                 overall_status = DB_MEM_ERROR;

                 free(temp_key_buffer); temp_key_buffer = NULL;
                 break; 
            }

            items_read = fread(temp_value_buffer, 1, vlen, file);
            if (items_read != vlen) {
                fprintf(stderr, "[ERROR] table_load: Failed to read %zu value bytes (read %zu) (Record %ld).\n", vlen, items_read, record_count + 1);
                overall_status = DB_FILE_CORRUPT; 

                free(temp_key_buffer); temp_key_buffer = NULL;
                free(temp_value_buffer); temp_value_buffer = NULL; 
                break; 
            }
            #if DEBUG_MODE
                fprintf(stderr, "[DEBUG] table_load: Record %ld: Read value data (%zu bytes).\n", record_count + 1, vlen);
            #endif
        } else {
             #if DEBUG_MODE
                fprintf(stderr, "[DEBUG] table_load: Record %ld: Value size is 0, skipping value read.\n", record_count + 1);
             #endif
        }

        #if DEBUG_MODE
            fprintf(stderr, "[DEBUG] table_load: Record %ld: Calling db_set for key '%s'...\n", record_count + 1, temp_key_buffer);
        #endif

        db_set_result = table_set(temp_key_buffer, temp_value_buffer, vlen, hashTable);

        if (db_set_result != DB_SUCCESS) {
            fprintf(stderr, "[ERROR] table_load: db_set failed for key '%s' (Error %d) (Record %ld).\n", temp_key_buffer, db_set_result, record_count + 1);
            overall_status = db_set_result; 

            free(temp_key_buffer); temp_key_buffer = NULL;
            free(temp_value_buffer); temp_value_buffer = NULL;
            break; 
        } else {
             record_count++;
             #if DEBUG_MODE
                fprintf(stderr, "[DEBUG] table_load: Record %ld: db_set successful.\n", record_count);
             #endif
        }

        free(temp_key_buffer); temp_key_buffer = NULL;
        free(temp_value_buffer); temp_value_buffer = NULL;

    } 

    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] table_load: Exited read loop. Final status before closing file: %d\n", overall_status);
    #endif

    free(temp_key_buffer);
    free(temp_value_buffer);

    if (file != NULL) {
        if (fclose(file) != 0) {
            fprintf(stderr, "[ERROR] table_load: Failed to close file '%s'\n", filename);

            if (overall_status == DB_SUCCESS) {
                overall_status = DB_FILE_ERROR;
            }
        } else {
             #if DEBUG_MODE
                fprintf(stderr, "[DEBUG] table_load: File '%s' closed successfully.\n", filename);
             #endif
        }
    }

    if (overall_status != DB_SUCCESS) {
         fprintf(stderr, "[WARN] table_load: Loading failed with status %d. Database may be incomplete.\n", overall_status);

    }

    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] table_load: Finished loading. Loaded %ld records. Returning status %d.\n", record_count, overall_status);
    #endif

    return overall_status;
}
