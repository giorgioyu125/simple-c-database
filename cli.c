// CLI_H
#include "cli.h" 

extern Node** hashTable;
extern HashSet* hashset;
char* db_type;
size_t db_size;

// CLI functionality

CommandType stocommand(const char* command_str, const char* db_type) {
    if (command_str == NULL) return CMD_UNKNOWN;

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
        if (strcmp(command_str, "INIT") == 0) return CMD_INIT;
    }

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
        if (strcmp(command_str, "INIT") == 0) return CMD_INIT;
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

int cmd_set_add(char* argument) {
    if (argument == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] set_add: Adding non-null key '%s'.\n", (const char*)argument);
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_add: Argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_add: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }
    
    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_add: Argument '%s' passed validation. Calling set_add.\n", (const char*)argument);
    #endif

    int error = set_add((unsigned char*)argument);
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_add: function failed execution because of set_add previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_add: Key '%s' added successfully.\n", (const char*)argument);
    #endif

    return CLI_SUCCESS;
}

int cmd_set_exist(char* argument, bool* existence) {
    if (argument == NULL) {
        fprintf(stderr, "[DEBUG] set_exist: Checking the existence of a NULL key '%s'.\n", (const char*)argument);
        return CLI_FAILURE;
    }
    
    if (strlen(argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_exist: Argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_exist: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_exist: Argument '%s' passed validation. Calling set_exist.\n", (const char*)argument);
    #endif

    int* error; 
    bool existence_value = set_exist((unsigned char*)argument, error);

    if (*error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_exist: Execution failed.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_exist: Key '%s' checked successfully.\n", (const char*)argument);
    #endif

    return CLI_SUCCESS;
}

int cmd_set_del(char* argument) {
    if (argument == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] cmd_set_del: Removing a null key can lead to bugs.\n");
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_del: Argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_del: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    int error = set_delete((unsigned char*)argument);
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_del: function failed execution because of set_del previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_del: Key '%s' deleted successfully.\n", (const char*)argument);
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_list_keys(char* argument) {
    if (argument == NULL) {
        fprintf(stderr, "[ERROR] set_list_keys: list_keys don't accept non-null key '%s'.\n", (const char*)argument);
        return CLI_FAILURE;
    }
    
    if (strlen(argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_list_keys: Argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_list_keys: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }
}

size_t cmd_set_count(void) {
    size_t counter = 0;
    if (strcmp(db_type, "SET") && hashset != NULL) {
        for (size_t i = 0; i < hashset->capacity; i++){
            if (hashset->buckets[i] != NULL) {
                counter++;
            } else {
                break;
            }
        }
        return counter;
    }

    if (strcmp(db_type, "TABLE")) {
        for (size_t j = 0; j < TABLE_SIZE; j++) {
            if (hashTable[j] != NULL) {
                counter++;
            } else {
                break;
            }
        }
        return counter;
    }

    return CLI_FAILURE;

}

int cmd_set_save(char* argument) {
    if (argument == NULL) {
        fprintf(stderr, "[ERROR] set_save: Saving to a incorrect filename '%s'.\n", (const char*)argument);
        return CLI_FAILURE;
    }
    
    size_t key_len = strlen(argument);
    if (key_len == 0) {
        fprintf(stderr, "[ERROR] cmd_set_save: Argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_save: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }
    

    int error = set_save(argument);
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_save: function failed execution because of set_save previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_save: Set saved in file '%s' successfully.\n", (const char*)argument);
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_load(char* argument) {
    if (argument == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] set_add: accessing a null file '%s' is not permissible.\n", (const char*)argument);
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_add: .\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_add: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }

    int error = set_save(argument);
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_save: function failed execution because of set_save previous call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_save: Set saved in file '%s' successfully.\n", (const char*)argument);
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_reset(char* argument) {
    if (argument == NULL) {
        #if DEBUG_MODE
            printf("[DEBUG] set_reset: Adding non-null key '%s'.\n", (const char*)argument);
        #endif
        return CLI_FAILURE;
    }
    
    if (strlen(argument) == 0) {
        fprintf(stderr, "[ERROR] cmd_set_reset: Argument (key) cannot be an empty string.\n");
        return CLI_FAILURE;
    }

    size_t key_len = strlen(argument);
    if (key_len > sizeof(size_t)) {
        fprintf(stderr, "[ERROR] cmd_set_reset: Key length (%zu) exceeds maximum allowed length (%zu).\n",
                key_len, sizeof(size_t));
        return CLI_FAILURE;
    }
    

    int error = set_destroy();
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_reset: function failed execution because of previous set_destroy call.\n");
        return CLI_FAILURE;
    }
    
    error = cmd_init((char*)db_size);
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_reset: set_init throw a problem while.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_reset: Set successfully resetted to full NULL state.\n");
    #endif
    
    return CLI_SUCCESS;
}

int cmd_set_exit(void) {
    int error = set_destroy();
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_set_exit: function failed execution because of previous set_destroy call.\n");
        return CLI_FAILURE;
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_set_exit: Set successfully destroyed to full NULL state.\n");
    #endif
    
    return CLI_SUCCESS;
}

// Table

int cmd_table_add(char *argument) {
    char *key_str;
    char *type_str;
    char *data_str;

    #if DEBUG_MODE
        printf("[DEBUG] cmd_table_add: Received argument: \"%s\"\n", argument);
    #endif

    key_str = argument;
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

    #if DEBUG_MODE
        printf("[DEBUG] cmd_table_add: Inferred type ID for '%s'.\n", type_str);
    #endif

    int error_db;

    switch (datatype) {
        case STR: { 
            error_db = db_set(key, data_str, strlen(data_str) + 1);
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set string value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }

            break;            
        }
        
        case INT: {
            char *endptr_int;
            long data_long = strtol(data_str, &endptr_int, 10); // Use strtol for better error checking

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
            error_db = db_set(key, &data_int, sizeof(int));
            if (error_db != DB_SUCCESS) {
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

            error_db = db_set(key, &converted_value, sizeof(converted_value));
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set hex value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }
            break;  
        }

        case FILENAME: {
            error_db = db_set(key, data_str, strlen(data_str) + 1);
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: Failed to set filename value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }
            #if DEBUG_MODE
                printf("[DEBUG] cmd_table_add: Stored filename '%s' for key '%s'.\n", data_str, key_str);
            #endif
            break; // Added break
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


int cmd_table_get(const unsigned char *argument) {
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_get: GET command requires a non-empty key.\n");
        return CLI_FAILURE;
    }

    size_t actual_value_size = 0; 
    void *value_ptr = NULL;       

    value_ptr = db_get(argument, &actual_value_size);

    if (value_ptr != NULL) {
        printf("Value (size %zu): ", actual_value_size);

        fwrite(value_ptr, 1, actual_value_size, stdout);
        printf("\n"); 
        return CLI_SUCCESS; 
    } else {
        fprintf(stderr, "[ERROR] cmd_table_get: Key '%s' not found.\n", argument);
        return CLI_FAILURE; 
    }
}
 
int cmd_table_del(const unsigned char *argument) {
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_get: GET command requires a non-empty key.\n");
        return CLI_FAILURE;
    }
  
    int error = db_delete(argument);
    
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_del: An error occured while deleting the key!\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
} 
 
int cmd_table_list_keys(void) {

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_list_keys: Database not initialized.\n");
        return CLI_FAILURE;
    }

    printf("--- Keys in Database ---\n");
    int key_count = 0; 

    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = hashTable[i];

        while (current != NULL) {

            if (current->key != NULL) { 
                printf("   %s\n", current->key);
                key_count++;
            } else {
                fprintf(stderr, "[WARN] cmd_table_list_keys: Found node with NULL key in bucket %d.\n", i);
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

int cmd_table_count(void) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_count: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int key_count = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = hashTable[i];

        while (current != NULL) {
            if (current->key != NULL){
                key_count++;
            } else {
                fprintf(stderr, "[ERROR] cmd_table_count: Found node with NULL key in bucket %d.\n", i);
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
 
int cmd_table_save(const char *argument) {
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_save: SAVE command requires a non-empty filename.\n");
        return CLI_FAILURE;
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_save: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int error = db_save(argument);
    
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_save: An error occured while invoking db_save.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_table_load(const char *argument) {
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_table_load: LOAD command requires a non-empty filename.\n");
        return CLI_FAILURE;
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_load: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int error = db_load(argument);
    
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_load: An error occured while invoking db_load.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_table_reset(void) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_reset: Database not initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = destroy_db();
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_reset: An error occured while destroying the database.\n");
        return CLI_SUCCESS;
    }

    error = init_db();
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_reset: An error occured while re-creating the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_help(void) {
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
    // Optional: Add COUNT <bucket_index> if you implemented that feature

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
 
int cmd_table_exit(void) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_table_exit: Database not initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = destroy_db();

    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_exit: An error occured while destroying the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_init(char* command_argument) {
    char* size_ptr = strchr(command_argument, ' ');
    db_size = (size_t)atol(size_ptr);


    if (strcmp(db_type, "TABLE") == 0) {
        int error = init_db(); // TODO: Add the size logic in the init_db function
    }

    if (strcmp(db_type, "SET") == 0) {
        int error = set_init(db_size);
    }
    
    int error = DB_FAILURE;

    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_table_exit: An error occured while initializing the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;   
}

void process_command(char *line) {
    #if DEBUG_MODE
        printf("[DEBUG] process_command: Starting line parsing...\n");
    #endif

    char *saveptr;
    char *command_str = strtok_r(line, " ", &saveptr);
    
    if (command_str == NULL) return;

    CommandType command = stocommand(command_str, db_type);
    char* command_argument = strtok_r(NULL, "", &saveptr); 

    #if DEBUG_MODE
        printf("[DEBUG] process_command: Line parsing finisched!\n");
        printf("[DEBUG] process_command: Starting command redirection...\n");
    #endif   

    switch (command) {
        case : {

        }
        
        case : {

        }

        case : {

        }

        case : {

        }

        case : {

        }

        case : {

        }

        case : {

        }

        case : {

        }

        case : {

        }

    CMD_SET_ADD
    CMD_SET_SET
    CMD_SET_GET
    CMD_SET_DEL
    CMD_SET_LIST_KEYS
    CMD_SET_COUNT
    CMD_SET_SAVE
    CMD_SET_LOAD
    CMD_SET_RESET
    CMD_SET_HELP
    CMD_SET_EXIT
    CMD_SET_QUIT

        // Table
        case CMD_TABLE_SET:
        case CMD_TABLE_ADD: {
            int error = cmd_table_add(command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }
        
        case CMD_TABLE_GET: {
            int error = cmd_table_get((const unsigned char*)command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while getting the value.\n");
                return;
            }
            return;
        }          
        
        case CMD_TABLE_DEL: {
            int error = cmd_table_del((const unsigned char*)command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the deletion of the value '%s'.\n", command_argument);
                return;
            }
            return;
        }

        case CMD_TABLE_LIST_KEYS: {
            int error = cmd_table_list_keys();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while listing the key-value pair of the table.\n");
                return;
            }
            return;
        }

        case CMD_TABLE_COUNT: {
            int error = cmd_table_count();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while counting the value in the Table.\n");
                return;
            }
            return;
        }

        case CMD_TABLE_SAVE: {
            int error = cmd_table_save(command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while saving the Table.\n");
                return;
            }
            return;
        }

        case CMD_TABLE_LOAD: {
            int error = cmd_table_load(command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while loading the saved Table.\n");
                return;
            }
            return;
        }

        case CMD_TABLE_RESET: {
            int error = cmd_table_reset();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while resetting the Table.\n");
                return;
            }
            return;
        }

        case CMD_TABLE_HELP: {
            int error = cmd_help();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while printing the help panel.\n");
                return;
            }
            return;
        }
        
        case CMD_TABLE_QUIT:
        case CMD_TABLE_EXIT: {
            int error = cmd_table_exit();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the EXIT functionality.\n");
                return;
            }
            return;
        }

        case CMD_INIT: {
            int error = cmd_init((const unsigned char*)command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_table_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_UNKNOWN: {
            fprintf(stderr, "[ERROR] process_command: Provided command is unknown, note that commands and arguments are case-sensitive.\n");
            return;
        }
    }

    #if DEBUG_MODE
        printf("[DEBUG] process_command: Command redirection finished!\n");
    #endif
}
