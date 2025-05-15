// Includes

#include <cstdint>
#include <cstdlib>
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uv.h>
#include "string_functionality.h"
#include "hashing_functions.h"

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

typedef struct Node{
    unsigned char *key;
    Node* next;
} Node;

typedef struct HashSet {
    Node** buckets;
    size_t capacity;
    size_t count;    
    uv_mutex_t mutex;
} HashSet;

HashSet* hashset = NULL;

// Hashset functionality

int set_init(const unsigned int capacity) {
    #if DEBUG_MODE
        printf("[DEBUG] set_init: Starting Hashset allocation.\n");
    #endif
    
    if (capacity == 0) {
        fprintf(stderr, "[ERROR] set_init: Selected capacity cannot be 0!\n");
    }
    
    hashset = (HashSet*)malloc(sizeof(HashSet));
    if (hashset == NULL) {
        fprintf(stderr, "[ERROR] set_init: Allocation failed!\n");
        return DB_MEM_ERROR;
    }

    hashset->capacity = capacity;
    hashset->count = 0;

    #if DEBUG_MODE
        printf("[DEBUG] set_init: Hashset allocation completed, starting buckets in Hashset allocation.\n");
    #endif

    hashset->buckets = (Node**)calloc(hashset->capacity, sizeof(Node*));
    if (hashset->buckets == NULL) {
        fprintf(stderr, "[ERROR] set_init: Impossibile to initialize buckets.\n");
        free(hashset);
        return DB_FAILURE;
    }
    
    if (uv_mutex_init(&hashset->mutex) != 0) {
        fprintf(stderr, "[ERROR] set_init: Errore nell'inizializzazione del mutex\n");
        free(hashset->buckets);
        free(hashset);
        return DB_FAILURE;
    }

    return DB_SUCCESS;
}

int set_destroy(void) {
    if (hashset == NULL) {
        return DB_SUCCESS;  
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_destroy: Starting HashSet destruction for set at %p.\n", (void*)hashset);
    #endif

    if (hashset->buckets != NULL) { 
        #if DEBUG_MODE
            printf("[DEBUG] set_destroy: Freeing cells and keys...\n");
        #endif

        for (size_t i = 0; i < hashset->capacity; ++i) {
            Node* current_node = hashset->buckets[i];

            while (current_node != NULL) {

                Node* target = current_node;
                current_node = current_node->next; 

                if (target->key != NULL) {
                    #if DEBUG_MODE
                        printf("[DEBUG] set_destroy: Freeing key for node at %p (bucket %zu): %s\n",
                               (void*)target, i, (const char*)target->key);
                    #endif

                    free(target->key);
                    target->key = NULL;
                }

                #if DEBUG_MODE
                    printf("[DEBUG] set_destroy: Freeing node struct at %p (bucket %zu).\n", (void*)target, i);
                #endif

                free(target);
            }

            hashset->buckets[i] = NULL;
        }

        #if DEBUG_MODE
            printf("[DEBUG] set_destroy: All cells and keys freed.\n");
            printf("[DEBUG] set_destroy: Freeing buckets array at %p.\n", (void*)hashset->buckets);
        #endif

        free(hashset->buckets);
        hashset->buckets = NULL;
    } else {
        #if DEBUG_MODE
            printf("[DEBUG] set_destroy: Buckets array was NULL (or capacity was 0).\n");
        #endif

        #if DEBUG_MODE
            printf("[DEBUG] set_destroy: Destroying mutex.\n");
        #endif
        uv_mutex_destroy(&hashset->mutex);

        #if DEBUG_MODE
            printf("[DEBUG] set_destroy: Freeing HashSet struct at %p.\n", (void*)hashset);
        #endif
        free(hashset);

        #if DEBUG_MODE
            printf("[DEBUG] set_destroy: HashSet destruction completed.\n");
        #endif
        return DB_SUCCESS;
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_destroy: Destroying mutex.\n");
    #endif
    
    uv_mutex_destroy(&hashset->mutex);  

    #if DEBUG_MODE
        printf("[DEBUG] set_destroy: Freeing HashSet struct at %p.\n", (void*)hashset);
    #endif

    free(hashset);

    #if DEBUG_MODE
        printf("[DEBUG] set_destroy: HashSet destruction completed.\n");
    #endif

    return DB_SUCCESS;
}

int set_add(unsigned char* key) {
    int errors = 0;

    if (hashset == NULL) {
        fprintf(stderr, "[ERROR] set_add: Critical error, Hashset is not initialized.\n");
        uv_mutex_unlock(&hashset->mutex);

        return DB_FAILURE;
    }
    
    if (key == NULL) {
        fprintf(stderr, "[ERROR] set_add: Key cannot be null.\n");
        errors += 1;
    }

    if (errors >= 1) {
        uv_mutex_unlock(&hashset->mutex);
        
        return DB_FAILURE;
    }
    
    uv_mutex_lock(&hashset->mutex);
    
    const unsigned long index = hash(key) % hashset->capacity;
    #if DEBUG_MODE
        printf("[DEBUG] set_add: key = %s, accessing to: hashset[%lu]", key, index);
    #endif

    Node* current = hashset->buckets[index];
    while (current != NULL) {

        if (ustrcmp(current->key, key) == 0) {

            #if DEBUG_MODE
                printf("[DEBUG] set_add: Key '%s' already exists in the bucket (index %lu).\n", (const char*)key, index);
            #endif
                uv_mutex_unlock(&hashset->mutex); 
                return DB_SUCCESS; 
        }
        current = current->next;
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "[ERROR] set_add: Allocation failed!\n");

        uv_mutex_unlock(&hashset->mutex);
        return DB_MEM_ERROR;
    }

    new_node->key = ustrdup(key); 
    if (new_node->key == NULL) {
        fprintf(stderr, "[ERROR] set_add: Failed to duplicate key '%s'.\n", (const char*)key);
        free(new_node);

        uv_mutex_unlock(&hashset->mutex);
        return DB_MEM_ERROR;
    } 

    new_node->next = hashset->buckets[index]; 
    hashset->buckets[index] = new_node;

    hashset->count++;
    
    #if DEBUG_MODE
        printf("[DEBUG] set_add: New node for key '%s' inserted successfully in bucket (index %lu).\n", (const char*)key, index);
        printf("[DEBUG] set_add: HashSet count is now %zu.\n", hashset->count);
    #endif

    uv_mutex_unlock(&hashset->mutex);
    return DB_SUCCESS;
}

int set_delete(unsigned char* key) {
    if (hashset == NULL) {
        fprintf(stderr, "[ERROR] set_delete: Critical error, HashSet is not initialized.\n");
        return DB_FAILURE; 
    }

    if (key == NULL) {
        fprintf(stderr, "[ERROR] set_delete: Key cannot be null.\n");
        return DB_FAILURE; 
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_delete: Attempting to delete key '%s'.\n", (const char*)key);
    #endif

    uv_mutex_lock(&hashset->mutex);

    const unsigned long index = hash(key) % hashset->capacity;

    #if DEBUG_MODE
        printf("[DEBUG] set_delete: Calculated index for key '%s' is %lu.\n", (const char*)key, index);
    #endif

    Node* current = hashset->buckets[index];
    Node* prev = NULL;

    while (current != NULL) {
        if (ustrcmp(current->key, key) == 0) {

            #if DEBUG_MODE
                printf("[DEBUG] set_delete: Key '%s' found at index %lu. Removing node.\n", (const char*)key, index);
            #endif

            if (prev == NULL) {
                hashset->buckets[index] = current->next;
            }

            else {
                prev->next = current->next;
            }

            if (current->key != NULL) {
                #if DEBUG_MODE
                    printf("[DEBUG] set_delete: Freeing key for node being deleted: '%s'.\n", (const char*)current->key);
                #endif

                free(current->key); 
                current->key = NULL;
            }

            #if DEBUG_MODE
                printf("[DEBUG] set_delete: Freeing node structure itself (ptr: %p).\n", (void*)current);
            #endif

            free(current);
            current = NULL; 

            hashset->count--;

            #if DEBUG_MODE
                printf("[DEBUG] set_delete: Key '%s' deleted successfully. HashSet count is now '%zu'.\n",
                       (const char*)key, hashset->count);
            #endif

            uv_mutex_unlock(&hashset->mutex);
            return DB_SUCCESS; 
        }

        prev = current;
        current = current->next;
    }

    #if DEBUG_MODE
        fprintf(stderr, "[DEBUG] set_delete: Key '%s' not found in HashSet.\n", (const char*)key);
    #endif

    uv_mutex_unlock(&hashset->mutex);
    return DB_KEY_NOT_FOUND; 
}

int set_save(const char* filename) {
    #if DEBUG_MODE
        printf("[DEBUG] set_save: Attempting to save HashSet to '%s'.\n", filename);
    #endif

    if (hashset == NULL) {
        fprintf(stderr, "[ERROR] set_save: HashSet is not initialized.\n");
        return DB_FAILURE; 
    }

    if (hashset->buckets == NULL && hashset->capacity > 0) {
        fprintf(stderr, "[ERROR] set_save: HashSet buckets are NULL but capacity is > 0.\n");
        return DB_FAILURE;
    }

    char temp_filename[FILENAME_MAX];
    int len = snprintf(temp_filename, sizeof(temp_filename), "%s.scdb", filename);
    if (len < 0 || (size_t)len >= sizeof(temp_filename)) {
        fprintf(stderr, "[ERROR] set_save: Failed to create temporary filename (too long or snprintf error).\n");
        return DB_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_save: Using temporary file '%s'.\n", temp_filename);
    #endif

    FILE *fp = fopen(temp_filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] set_save: Failed to open the file, probably permissions are missing!\n");
        return DB_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_save: Temporary file '%s' opened successfully for writing.\n", temp_filename);
    #endif

    #if DEBUG_MODE
        printf("[DEBUG] set_save: Starting iteration through HashSet (capacity %zu).\n", hashset->capacity);
        unsigned long keys_written = 0;
    #endif

    uv_mutex_lock(&hashset->mutex);

    for (size_t i = 0; i < hashset->capacity; i++) {
        Node *current = hashset->buckets[i];
        #if DEBUG_MODE
            if (current != NULL) {
                printf("[DEBUG] set_save: Processing bucket %zu.\n", i);
            }
        #endif

        while (current != NULL) {
            if (current->key == NULL) {
                fprintf(stderr, "[WARNING] set_save: Found node with NULL key in bucket %zu. Skipping.\n", i);
                current = current->next;
                continue;
            }

            size_t klen = strlen((const char*)current->key) + 1;
            size_t items_written;

            #if DEBUG_MODE
                printf("[DEBUG] set_save: Writing key: '%s' (klen=%zu)\n",
                        (const char*)current->key, klen);
            #endif

            items_written = fwrite(&klen, sizeof(size_t), 1, fp);
            if (items_written != 1) {
                goto write_error;
            }

            items_written = fwrite(current->key, 1, klen, fp);
            if (items_written != klen) {
                goto write_error;
            }

            #if DEBUG_MODE
                keys_written++;
            #endif

            current = current->next;
        }
    }

    uv_mutex_unlock(&hashset->mutex);

    #if DEBUG_MODE
        printf("[DEBUG] set_save: Finished set iteration. %lu keys written to temporary file.\n", keys_written);
    #endif

    if (fclose(fp) == EOF) {
        fp = NULL;
        fprintf(stderr, "[ERROR] set_save: Failed to close temporary file after writing, file: '%s'\n", temp_filename);
        remove(temp_filename);
        return DB_FAILURE;
    }
    fp = NULL;
    #if DEBUG_MODE
        printf("[DEBUG] set_save: Temporary file '%s' closed successfully.\n", temp_filename);
    #endif

    if (rename(temp_filename, filename) != 0) {
        fprintf(stderr, "[ERROR] set_save: Failed to rename temporary file to final file, from: '%s' To: '%s'\n", temp_filename, filename);
        int remove_error = remove(temp_filename);
        if (remove_error != 0) {
            #if DEBUG_MODE
                printf("[ERROR] set_save: Failed to remove the file!\n");
            #endif
        }
        return DB_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_save: Successfully renamed '%s' to '%s'. Save complete.\n", temp_filename, filename);
    #endif

    return DB_SUCCESS;

write_error:
    #if DEBUG_MODE
        printf("[DEBUG] set_save: Entering write_error handler. Error occurred writing to '%s'.\n", temp_filename);
    #endif

    uv_mutex_unlock(&hashset->mutex);

    fprintf(stderr, "[ERROR] set_save: Error writing to temporary file '%s'.\n", temp_filename);
    if (fp != NULL) {
        fclose(fp);
    }

    remove(temp_filename);
    return DB_FAILURE;
}

int set_load(const char *filename) {
    if (hashset == NULL) {
        return DB_FAILURE;
    }
    
    #if !LOAD_OVERLAP
        set_destroy();
    #endif

    #if DEBUG_MODE
        printf("[DEBUG] hashset_load: Attempting to load HashSet from '%s' with initial_capacity %zu.\n",
                filename, hashset->capacity);
    #endif

    FILE *fp = fopen(filename, "rb"); 
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] set_load: An errro occured when openning the file. Filename: '%s'\n", filename);
        return DB_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] set_load: File '%s' opened successfully for binary reading.\n", filename);
    #endif

    #if DEBUG_MODE
        printf("[DEBUG] set_load: Starting to count entries from file.\n");
        unsigned long entries_read_count = 0;
    #endif

    while (true) {
        size_t klen = 0;
        unsigned char *key_buffer = NULL;
        size_t items_read;

        items_read = fread(&klen, sizeof(size_t), 1, fp);
        if (items_read != 1) {
            if (feof(fp)) { // Loop escape condition
                #if DEBUG_MODE
                    printf("[DEBUG] set_load: End of file reached (expected while reading klen). Load successful.\n");
                #endif
                break; 
            } else {
                fprintf(stderr, "[ERROR] set_load: Failed to read klen from file.\n");
                goto load_error;
            }
        }

        if (klen == 0 || klen > SIZE_MAX) { 
            fprintf(stderr, "[ERROR] set_load: Invalid klen read: %zu. File likely corrupt or key too long.\n", klen);
            goto load_error;
        }

        key_buffer = (unsigned char *)malloc(klen);
        if (key_buffer == NULL) {
            fprintf(stderr, "[ERROR] set_load: Failed to allocate memory for key (klen=%zu).\n", klen);
            goto load_error; 
        }

        items_read = fread(key_buffer, 1, klen, fp);
        if (items_read != klen) {
            fprintf(stderr, "[ERROR] set_load: Failed to read full key data (expected %zu, got %zu).\n", klen, items_read);
            free(key_buffer); 
            goto load_error;
        }
        #if DEBUG_MODE
            printf("[DEBUG] set_load: Read key: '%.*s' (klen=%zu)\n", (int)klen, (const char*)key_buffer, klen);
        #endif

        int set_error = set_add(key_buffer);
        free(key_buffer);

        if (set_error != DB_SUCCESS) {
            fprintf(stderr, "[ERROR] set_load: Failed to add new node (key: '%.*s') to HashSet structure.\n", (int)klen, (const char*)key_buffer);

            goto load_error;
        }

        #if DEBUG_MODE
            entries_read_count++;
        #endif
    } 

    #if DEBUG_MODE
        printf("[DEBUG] set_load: Finished reading file. %lu entries loaded into HashSet.\n", entries_read_count);
    #endif

    fclose(fp);
    return DB_SUCCESS; 

load_error:
    #if DEBUG_MODE
        printf("[DEBUG] set_load: Entering load_error_cleanup.\n");
    #endif
    if (fp != NULL) fclose(fp);
    return DB_FAILURE; 
}

bool set_exist(const unsigned char* key, int* out_error_value) {
    if (key == NULL) {
        fprintf(stderr, "[ERROR] set_exist: the provided key is NULL.\n");
        *out_error_value = DB_FAILURE;
        return false;
    }

    const unsigned long index = hash(key) % hashset->capacity;
    
    #if DEBUG_MODE
        printf("[DEBUG] set_exist: Index calculated '%lu'.\n", index);
    #endif
    
    int address_pos = 0;
    Node* current = hashset->buckets[index];

    while (current != NULL) {
        #if DEBUG_MODE 
            printf("[DEBUG] set_exist: starting string comparison with the key in address position '%d'...", address_pos);
        #endif

        if (ustrcmp(key, current->key) == 0) {
            *out_error_value = DB_SUCCESS;
            return true;
        }
        current = current->next;
    }
    #if DEBUG_MODE
        printf("[DEBUG] set_exist: Selected key doesnt exist!!\n");
    #endif

    *out_error_value = DB_SUCCESS;
    return false;
}
