// CLI_H

#include "cli.h" 
#include "hashset.h"

// CLI functionality

// refactor THIS with switch case
CommandType stocommand(const char* command_str, const char* db_type) {
    if (command_str == NULL) return CMD_UNKNOWN;
    if (db_type == NULL) return CMD_UNKNOWN;

    if (strcmp(db_type, "SET") == 0) {
        if (strcmp(command_str, "ADD") == 0 || strcmp(command_str, "SET") == 0) return CMD_SET_ADD;
        if (strcmp(command_str, "GET") == 0) return CMD_SET_GET;
        if (strcmp(command_str, "DEL") == 0) return CMD_SET_DEL;
        if (strcmp(command_str, "LIST-KEYS") == 0) return CMD_SET_LIST_KEYS;
        if (strcmp(command_str, "COUNT") == 0) return CMD_SET_COUNT;
        if (strcmp(command_str, "SAVE") == 0) return CMD_SET_SAVE;
        if (strcmp(command_str, "LOAD") == 0) return CMD_SET_LOAD;
        if (strcmp(command_str, "RESET") == 0) return CMD_SET_RESET;
        if (strcmp(command_str, "HELP") == 0) return CMD_SET_HELP;
        if (strcmp(command_str, "EXIT") == 0 || strcmp(command_str, "QUIT") == 0) return CMD_SET_EXIT;
        if (strcmp(command_str, "INIT") == 0) return CMD_SET_INIT;
    }


    if (strcmp(db_type, "TABLE") == 0) {
        if (strcmp(command_str, "ADD") == 0 || strcmp(command_str, "SET") == 0) return CMD_TABLE_ADD;
        if (strcmp(command_str, "GET") == 0) return CMD_TABLE_GET;
        if (strcmp(command_str, "DEL") == 0) return CMD_TABLE_DEL;
        if (strcmp(command_str, "LIST-KEYS") == 0) return CMD_TABLE_LIST_KEYS;
        if (strcmp(command_str, "COUNT") == 0) return CMD_TABLE_COUNT;
        if (strcmp(command_str, "SAVE") == 0) return CMD_TABLE_SAVE;
        if (strcmp(command_str, "LOAD") == 0) return CMD_TABLE_LOAD;
        if (strcmp(command_str, "RESET") == 0) return CMD_TABLE_RESET;
        if (strcmp(command_str, "HELP") == 0) return CMD_TABLE_HELP;
        if (strcmp(command_str, "EXIT") == 0 || strcmp(command_str, "QUIT") == 0) return CMD_TABLE_EXIT;
        if (strcmp(command_str, "INIT") == 0) return CMD_TABLE_INIT;
    }

    return CMD_UNKNOWN;
}

DataTypes stodatatype(const char *datatype_str) {
    if (datatype_str == NULL) return DATATYPE_UNKNOWN;

    if (strcmp(datatype_str, "STR") == 0) return STR;
    if (strcmp(datatype_str, "INT") == 0) return INT;
    if (strcmp(datatype_str, "HEX") == 0) return HEX;
    if (strcmp(datatype_str, "FILE") == 0) return FILENAME;

    return DATATYPE_UNKNOWN;
}

// Set

int cmd_set_add(char* command_argument, HashSet* hashset) {
    if (command_argument == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] set_add: Adding a Null key is impossible.\n");
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(command_argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_add: command_argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(command_argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_add: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }
    
    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_add: command_argument '%s' passed validation. Calling set_add.\n", (const char*)command_argument);
    #endif

    int error = set_add((unsigned char*)command_argument, hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_add: function failed execution because of set_add previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_add: Key '%s' added successfully.\n", (const char*)command_argument);
    #endif

    return CLI_SUCCESS;
}

int cmd_set_exist(char* command_argument, bool* existence, HashSet* hashset) {
    if (command_argument == NULL) {
        fprintf(stderr, "[ERROR] cmd_set_exist: command_argument (key) cannot be NULL.\n");
        return CLI_FAILURE;
    }

    if (strlen(command_argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_exist: command_argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(command_argument);
    if (key_len > sizeof(size_t)) { 
        fprintf(stderr, "[ERROR] cmd_set_exist: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_exist: command_argument '%s' passed validation. Calling set_exist.\n", (const char*)command_argument);
    #endif

    int db_operation_error_code; 
    bool key_exists_value;       

    key_exists_value = set_exist((const unsigned char*)command_argument, &db_operation_error_code, hashset);

    if (db_operation_error_code != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_exist: Underlying set_exist function failed (error code: %d).\n", db_operation_error_code);
        return CLI_FAILURE;
    }

    if (existence == NULL) {
        fprintf(stderr, "[ERROR] cmd_set_exist: Output parameter 'existence' is NULL. Cannot store result.\n");
        return CLI_FAILURE; 
    }
    *existence = key_exists_value;

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_exist: Key '%s' checked successfully. Existence: %s.\n",
               (const char*)command_argument, key_exists_value ? "true" : "false");
    #endif

    return CLI_SUCCESS;
}

int cmd_set_del(char* command_argument, HashSet* hashset) {
    if (hashset == NULL) {
        fprintf(stderr, "[ERROR] cmd_set_del: Hashset is not initialized correctly.\n");
        #if DEBUG_MODE
            printf("[DEBUG] cmd_set_del: Hashset pointer is NULL.\n");
        #endif
        return CLI_FAILURE;
    }


    if (command_argument == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] cmd_set_del: Removing a null key can lead to bugs.\n");
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(command_argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_del: command_argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(command_argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_del: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    int error = set_delete((unsigned char*)command_argument, hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_del: function failed execution because of set_del previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_del: Key '%s' deleted successfully.\n", (const char*)command_argument);
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_list_keys(HashSet* hashset) { 
    if (hashset == NULL) {
        fprintf(stderr, "[ERROR] cmd_set_list_keys: Database (hashset) is not initialized!\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_list_keys: Database type is 'SET'. Listing keys...\n");
    #endif
    size_t keys_found = 0;
    for (size_t i = 0; i < hashset->capacity; i++) {
        if ((hashset->buckets[i] != NULL) && (hashset->buckets[i]->key != NULL)) { 
            printf("Key n.%zu -> %s\n", i, hashset->buckets[i]->key);
            keys_found++;
        }
    }

    if (keys_found == 0) {
        printf("(No keys found in the SET database)\n");
    }
    return CLI_SUCCESS;
}

int cmd_set_count(HashSet* hashset) {
    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_count: Database type is 'SET'. Listing all keys...\n");
    #endif

    size_t counter = 0;

    for (size_t i = 0; i < hashset->capacity; i++) {
        if (hashset->buckets[i] != NULL) {
            counter++;
        } 
    }
    printf("[INFO] Non-null Keys number is: %zu/%zu", counter, hashset->capacity);
    return CLI_SUCCESS;

    fprintf(stderr, "[ERROR] cmd_set_count: Wrong type of db, a fatal error occured!!\n");
    return CLI_FAILURE;
}

int cmd_set_save(char* filename, HashSet* hashset) {
    if (filename == NULL) {
        fprintf(stderr, "[ERROR] cmd_set_save: Saving to a empty filename.\n");
        return CLI_FAILURE;
    }
    
    size_t key_len = strlen(filename);
    if (key_len == 0) {
        fprintf(stderr, "[ERROR] cmd_set_save: filename (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    if (key_len > MAX_FILENAME_LENGHT) {
        fprintf(stderr, "[ERROR] cmd_set_save: filename length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    int error = set_save(filename, hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_save: Function failed execution because of set_save previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_save: Set saved in file '%s' successfully.\n", (const char*)filename);
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_load(char* filename, HashSet* hashset) {
    if (filename == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] cmd_set_load: accessing a null file is not permissible.\n");
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(filename) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_load: filename need to have a proper lenght (>1).\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(filename);
    if (key_len > MAX_FILENAME_LENGHT) {
        fprintf(stderr, "[ERROR] cmd_set_load: filename length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    int error = set_save(filename, hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_load: Function failed execution because of set_save previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_load: Set saved in file '%s' successfully.\n", (const char*)filename);
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_reset(char* command_argument, size_t db_size, HashSet* hashset) {
    int error = set_destroy(hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_reset: function failed execution because of previous set_destroy call.\n");
        return CLI_FAILURE;
    }

    error = cmd_set_init(command_argument, db_size, hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_reset: set_init throw a problem while.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_reset: Set successfully resetted to full NULL state.\n");
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_exit(HashSet* hashset) {
    int error = set_destroy(hashset);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_exit: function failed execution because of previous set_destroy call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_exit: Set successfully destroyed to full NULL state.\n");
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_help(void) {
    const char *help_lines[] = {
        "SET Subsystem Help:\n",
        "----------------------------------------------------------------------\n",
        "This subsystem provides commands for managing a key-value store.\n",
        "The behavior is similar to a 'set' data structure for unique keys.\n",
        "\n",
        "Available commands:\n",
        "  SET <key> <value>      - Stores the given string value associated with the string key.\n",
        "                           If the key already exists, its value is updated.\n",
        "                           Example: SET server:port 8080\n",
        "\n",
        "  GET <key>              - Retrieves and displays the string value associated with the key.\n",
        "                           Example: GET server:port\n",
        "\n",
        "  DEL <key>              - Deletes the key and its associated value from the store.\n",
        "                           Example: DEL server:port\n",
        "\n",
        "  LISTKEYS <command_argument>    - Lists all keys currently stored.\n",
        "                           Note: The <command_argument> is syntactically required but may not be used\n",
        "                           for filtering in current implementations.\n",
        "                           Example: LISTKEYS any_text_here\n",
        "\n",
        "  COUNT                  - Displays the total number of key-value pairs currently stored.\n",
        "                           Example: COUNT\n",
        "\n",
        "  SAVE <filename>        - Saves the current state of the store to the specified file.\n",
        "                           Example: SAVE my_data.db\n",
        "\n",
        "  LOAD <filename>        - Loads the store's state from the specified file.\n",
        "                           Warning: This may overwrite any data currently in memory.\n",
        "                           Example: LOAD my_data.db\n",
        "\n",
        "  HELP                   - Displays this help message for the SET subsystem.\n",
        "----------------------------------------------------------------------\n",
        "General Notes:\n",
        "  - Keys are typically unique identifiers.\n",
        "  - If keys or values contain spaces or special characters, they might need\n",
        "    to be enclosed in quotes, depending on your command-line parser.\n",
        "    (e.g., SET \"user name\" \"Jane Doe\")\n",
        NULL 
    };
    for (int i = 0; help_lines[i] != NULL; ++i) {
        if (printf("%s", help_lines[i]) < 0) {
            return CLI_FAILURE;
        }
    }
    return CLI_SUCCESS;
}

int cmd_set_init(char* command_argument, size_t db_size, HashSet* out_hashset) {
    db_size = (size_t)atol(command_argument);

    out_hashset = NULL; 
    int error = set_init(db_size, &out_hashset);
    
    if (error == CLI_FAILURE) {
        fprintf(stderr, "[ERROR] cmd_table_init: An error occured while initializing or selecting the size of the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;     
}

// Table

int cmd_table_add(char *command_argument, Node** hashtable) {
    char *key_str;
    char *type_str;
    char *data_str;

    #if DEBUG_MODE
        printf("[DEBUG] cmd_table_add: Received command_argument: \"%s\"\n", command_argument);
    #endif

    key_str = command_argument;
    char *first_space = strchr(key_str, ' ');
    if (first_space == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_add: Invalid format. Expected 'KEY DATATYPE:VALUE'. Missing space after key.\n");
        return CLI_FAILURE;
    }
    *first_space = '\0'; 

    type_str = first_space + 1;
    while (*type_str && isspace((unsigned char)*type_str)) { 
        type_str++;
    }
    if (*type_str == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_add: Invalid format. Expected 'DATATYPE:VALUE' after key. Missing type.\n");
        return CLI_FAILURE;
    }

    char *first_colon = strchr(type_str, ':');
    if (first_colon == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_add: Invalid format. Expected 'DATATYPE:VALUE'. Missing colon after type.\n");
        return CLI_FAILURE;
    }
    *first_colon = '\0'; 

    data_str = first_colon + 1;

    #if DEBUG_MODE
        printf("[DEBUG] cmd_table_add: Parsed: Key='%s', Type='%s', Data='%s'\n", key_str, type_str, data_str);
    #endif

    const unsigned char *key = (const unsigned char*)key_str;
    DataTypes datatype = stodatatype(type_str);
    if (datatype == DATATYPE_UNKNOWN) datatype = STR;
    
    #if DEBUG_MODE
        printf("[DEBUG] cmd_table_add: Inferred type ID for '%s'.\n", type_str);
    #endif

    int error_db;

    switch (datatype) {
        case STR: { 
            error_db = table_set(key, data_str, strlen(data_str) + 1, hashtable);
            if (error_db != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set string value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }

            break;            
        }
        
        case INT: {
            char *endptr_int;
            long data_long = strtol(data_str, &endptr_int, 10);

            if (endptr_int == data_str) {
                fprintf(stderr, "[ERROR] cmd_table_add: Invalid integer format for '%s'. No digits found.\n", data_str);
                return CLI_FAILURE;
            }
            while (*endptr_int != '\0' && isspace((unsigned char)*endptr_int)) {
                endptr_int++;
            }
            if (*endptr_int != '\0') {
                fprintf(stderr, "[ERROR] cmd_table_add: Invalid characters ('%s') found after integer in '%s'.\n", endptr_int, data_str);
                return CLI_FAILURE;
            }
            if ((data_long == LONG_MAX || data_long == LONG_MIN) || data_long > INT_MAX || data_long < INT_MIN) {
                fprintf(stderr, "[ERROR] cmd_table_add: Integer value '%s' is out of range for type int.\n", data_str);
                return CLI_FAILURE;
            }
            
            int data_int = (int)data_long;
            error_db = table_set(key, &data_int, sizeof(int), hashtable);
            if (error_db != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set int value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }

            break;
        }

        case HEX: {
            char *endptr_hex;
            unsigned long converted_value = strtoul(data_str, &endptr_hex, 16);

            if (converted_value == ULONG_MAX) {
                fprintf(stderr, "[ERROR] cmd_table_add: Hex value '%s' is too large (overflow) for unsigned long.\n", data_str);
                return CLI_FAILURE;
            }

            if (endptr_hex == data_str) {
                fprintf(stderr, "[ERROR] cmd_table_add: Invalid hexadecimal format for '%s'. No valid hex digits found.\n", data_str);
                return CLI_FAILURE;
            }

            char *checkptr = endptr_hex;
            while (*checkptr != '\0' && isspace((unsigned char)*checkptr)) {
                checkptr++;
            }
            if (*checkptr != '\0') {
                fprintf(stderr, "[ERROR] cmd_table_add: Invalid characters ('%s') found after hexadecimal number in '%s'.\n", checkptr, data_str);
                return CLI_FAILURE;
            }

            error_db = table_set(key, &converted_value, sizeof(converted_value), hashtable);
            if (error_db != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set hex value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }
            break;  
        }

        case FILENAME: {
            error_db = table_set(key, data_str, strlen(data_str) + 1, hashtable);
            if (error_db != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set filename value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }
            #if DEBUG_MODE
                printf("[DEBUG] cmd_table_add: Stored filename '%s' for key '%s'.\n", data_str, key_str);
            #endif
            break; 
        }

        case DATATYPE_UNKNOWN:
        default: {
            fprintf(stderr, "[ERROR] cmd_table_add: Please use a known datatype annotation! And remember is case-sensitive.\n");
            return CLI_FAILURE;
        }
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_table_add: Successfully processed ADD command for key '%s'.\n", key_str);
    #endif

    return CLI_SUCCESS;
}


int cmd_table_get(const unsigned char *command_argument, Node** hashtable) {
    if (command_argument == NULL || *command_argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_get: GET command requires a non-empty key.\n");
        return CLI_FAILURE;
    }

    size_t actual_value_size = 0; 
    void *value_ptr = NULL;       

    value_ptr = table_get(command_argument, &actual_value_size, hashtable);

    if (value_ptr != NULL) {
        printf("Value (size %zu): ", actual_value_size);

        fwrite(value_ptr, 1, actual_value_size, stdout);
        printf("\n"); 
        return CLI_SUCCESS; 
    } else {
        fprintf(stderr, "[ERROR] cmd_table_get: Key '%s' not found.\n", command_argument);
        return CLI_FAILURE; 
    }
}

int cmd_table_del(const unsigned char *command_argument, Node** hashtable) {
    if (command_argument == NULL || *command_argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_del: GET command requires a non-empty key.\n");
        return CLI_FAILURE;
    }
  
    int error = table_delete(command_argument, hashtable);
    
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_del: An error occured while deleting the key!\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
} 

int cmd_table_list_keys(size_t db_size, Node** hashTable) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_list_keys: Database not initialized.\n");
        return CLI_FAILURE;
    }

    printf("--- Keys in Database ---\n");
    int key_count = 0; 

    for (size_t i = 0; i < db_size; i++) {
        Node *current = hashTable[i];
        while (current != NULL) {
            if (current->key != NULL) { 
                printf("   %s\n", current->key);
                key_count++;
            } else {
                fprintf(stderr, "[WARN] cmd_table_list_keys: Found node with NULL key in bucket %zu.\n", i);
            }
            current = current->next;
        }
    }

    if (key_count == 0) {
        printf("  (No keys found)\n");
    }
    printf("------------------------\n");

    return CLI_SUCCESS;
}

int cmd_table_count(size_t db_size, Node** hashTable) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_count: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int key_count = 0;

    for (size_t i = 0; i < db_size; i++) {
        Node *current = hashTable[i];

        while (current != NULL) {
            if (current->key != NULL){
                key_count++;
            } else {
                #if DEBUG_MODE
                    printf("[INFO] cmd_table_count: Found node with NULL key in bucket %zu.\n", i); // Maybe too invasive, whatever.
                #endif
            }
        }
    }

    if (key_count == 0) {
        printf("[INFO] cmd_table_count: No keys found!\n");
    } else {
        printf("Total key found in hashTable -> %d", key_count);
    }

    return CLI_SUCCESS;
}
 
int cmd_table_save(const char *command_argument, Node** hashTable) {
    if (command_argument == NULL || *command_argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_save: SAVE command requires a non-empty filename.\n");
        return CLI_FAILURE;
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_save: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int error = table_save(command_argument, hashTable);
    
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_save: An error occured while invoking db_save.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_table_load(const char *command_argument, size_t db_size, Node** hashTable) {
    if (command_argument == NULL || *command_argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_load: LOAD command requires a non-empty filename.\n");
        return CLI_FAILURE;
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_load: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int error = table_load(command_argument, db_size, hashTable);
    
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_load: An error occured while invoking db_load.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_table_reset(char* command_argument, size_t db_size, Node** hashTable) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_reset: Database not initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = table_destroy(hashTable);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_reset: An error occured while destroying the database.\n");
        return CLI_FAILURE;
    }

    error = cmd_table_init(command_argument, db_size, hashTable);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_reset: An error occured while re-creating the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_table_help(void) {
    printf("\n--- Simple C Database Help ---\n");
    printf("Available commands (case-sensitive):\n\n");

    printf("  ADD <key> <datatype:data>\n");
    printf("  SET <key> <datatype:data>\n");
    printf("      Adds or updates the value associated with the <key>.\n");
    printf("      <value_representation> can be:\n");
    printf("        - STR: text value\n");
    printf("        - HEX:   hex:<hex_digits> (e.g., hex:48656C6C6F)\n");
    printf("        - FILE:  file:<filepath> (e.g., file:/path/to/data.bin)\n");
    printf("        - INT:    int:<number> (e.g., int:12345)\n");
    printf("      (If no prefix is used, value is treated as plain text 'STR')\n\n");

    printf("  GET <key>\n");
    printf("      Retrieves and prints the value associated with the <key>.\n");
    printf("      Output format may vary for binary data.\n\n");

    printf("  DEL <key>\n");
    printf("      Deletes the key and its associated value from the database.\n\n");

    printf("  LIST-KEYS\n");
    printf("      Lists all keys currently stored in the database.\n\n");

    printf("  COUNT\n");
    printf("      Displays the total number of key-value pairs in the database.\n\n");

    printf("  SAVE <filename>\n");
    printf("      Saves the current state of the in-memory database to the specified file.\n\n");

    printf("  LOAD <filename>\n");
    printf("      Loads the database state from the specified file.\n");
    printf("      WARNING: This clears any data currently in memory before loading.\n\n");

    printf("  RESET\n");
    printf("      Removes all data from the in-memory database.\n");
    printf("      (You might be asked for confirmation).\n\n");

    printf("  HELP\n");
    printf("      Displays this help message.\n\n");

    printf("  EXIT\n");
    printf("  QUIT\n");
    printf("      Exits the application.\n\n");

    printf("----------------------------\n");

    return CLI_SUCCESS;
}
 
int cmd_table_exit(Node** hashTable) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_exit: Database not initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = table_destroy(hashTable);
    if (error != CLI_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_exit: An error occured while destroying the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_table_init(char* command_argument, size_t db_size, Node** out_hashtable) {
    db_size = (size_t)atol(command_argument);
    int error = table_init(db_size, out_hashtable);
    
    if (error == CLI_FAILURE) {
        fprintf(stderr, "[ERROR] cmd_table_init: An error occured while initializing or selecting the size of the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

// Command Processor

int process_command(char *line, char* db_type, size_t db_size, void* out_db) {
    #if DEBUG_MODE
        printf("[DEBUG] process_command: Starting line parsing...\n");
    #endif    


    char *saveptr;
    char *command_str = strtok_r(line, " ", &saveptr);
    
    if (command_str == NULL) return CLI_FAILURE;

    CommandType command = stocommand(command_str, db_type);
    char* command_argument = strtok_r(NULL, "", &saveptr); 

    #if DEBUG_MODE
        printf("[DEBUG] process_command: Line parsing finished!\n");
        printf("[DEBUG] process_command: Starting command redirection...\n");
    #endif   

    switch (command) {
        case CMD_SET_ADD: {
            int error = cmd_set_add(command_argument, out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_add execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_add function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }
        
        case CMD_SET_SET: {
            int error = cmd_set_add(command_argument, out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_add execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_add function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }

        case CMD_SET_GET: {
            bool result;
            int error = cmd_set_exist(command_argument, &result, out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_get execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_get function.\n");
                return CLI_FAILURE;
            }

            return result;
        }

        case CMD_SET_DEL: {
            int error = cmd_set_del(command_argument, out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_del execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_del function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }

        case CMD_SET_LIST_KEYS: {
            int error = cmd_set_list_keys(out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_list_keys execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_list_keys function.\n");
                return CLI_FAILURE;
            }
            
            return CLI_SUCCESS;
        }

        case CMD_SET_COUNT: {
            int error = cmd_set_count(out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_count execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_count function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }

        case CMD_SET_SAVE: {
            int error = cmd_set_save(command_argument, out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_save execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_save function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }

        case CMD_SET_LOAD: {
            int error = cmd_set_load(command_argument, out_db);
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_load execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_load function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }

        case CMD_SET_RESET: {
            int error = cmd_set_reset(command_argument, db_size, out_db);

            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_reset execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_reset function.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_SET_EXIT: 
        case CMD_SET_QUIT: {
            db_size = 0;
            db_type = NULL;
            int error = cmd_set_exit(out_db);

            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_quit execution finished.\n");
            #endif

            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_quit function.\n");
                return CLI_FAILURE;
            }

            return CLI_SUCCESS;
        }

        case CMD_SET_HELP: {
            int error = cmd_set_help();
            #if DEBUG_MODE
                printf("[DEBUG] process_command: cmd_set_help execution finished.\n");
            #endif
            
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] process_command: An error occured while executing the cmd_set_help function.\n");
                return CLI_FAILURE;
            }
            
            return CLI_SUCCESS;
        }

        case CMD_SET_INIT: {
            int error = cmd_set_init(command_argument, db_size, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_set_init: There was an error while executing the insertion of a new value.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        // Table
        case CMD_TABLE_SET:
        case CMD_TABLE_ADD: {
            int error = cmd_table_add(command_argument, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the insertion of a new value.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }
        
        case CMD_TABLE_GET: {
            int error = cmd_table_get((const unsigned char*)command_argument, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while getting the value.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }          
        
        case CMD_TABLE_DEL: {
            int error = cmd_table_del((const unsigned char*)command_argument, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the deletion of the value '%s'.\n", command_argument);
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_LIST_KEYS: {
            int error = cmd_table_list_keys(db_size, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while listing the key-value pair of the table.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_COUNT: {
            int error = cmd_table_count(db_size, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while counting the value in the Table.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_SAVE: {
            int error = cmd_table_save(command_argument, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while saving the Table.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_LOAD: {
            int error = cmd_table_load(command_argument, db_size, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while loading the saved Table.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_RESET: {
            int error = cmd_table_reset(command_argument, db_size, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while resetting the Table.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_HELP: {
            int error = cmd_table_help();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while printing the help panel.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }
        
        case CMD_TABLE_QUIT:
        case CMD_TABLE_EXIT: {
            db_size = 0;
            db_type = NULL;
            int error = cmd_table_exit(out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the EXIT functionality.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        case CMD_TABLE_INIT: {
            int error = cmd_table_init(command_argument, db_size, out_db);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the insertion of a new value.\n");
                return CLI_FAILURE;
            }
            return CLI_SUCCESS;
        }

        default:
        case CMD_UNKNOWN: {
            fprintf(stderr, "[ERROR] process_command: Provided command is unknown, note that commands and arguments are case-sensitive.\n");
            return CLI_SUCCESS;
        }
    }

    #if DEBUG_MODE
        printf("[DEBUG] process_command: Command redirection finished!\n");
    #endif
}
