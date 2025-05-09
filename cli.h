// Includes

#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "key-value_db.h"

// Debug Mode

#define DEGUB_MODE 0

// Error handling

#define CLI_SUCCESS 0
#define DB_SUCCESS 0
#define DB_FAILURE -1       
#define DB_KEY_NOT_FOUND -2
#define DB_MEM_ERROR -3     
#define DB_KEY_EXISTS -4   
#define DB_FILE_CORRUPT -5
#define DB_FILE_ERROR -6
#define CLI_FAILURE -7

// Data

#define TABLE_SIZE 4096

typedef enum {
    STR,
    INT,
    HEX,
    FILENAME,
    DATATYPE_UNKNOWN
} DataTypes;


typedef enum {
    CMD_ADD,
    CMD_SET,
    CMD_GET,
    CMD_DEL,
    CMD_LIST_KEYS,
    CMD_COUNT,
    CMD_SAVE,
    CMD_LOAD,
    CMD_RESET,
    CMD_HELP,
    CMD_EXIT,
    CMD_QUIT,
    CMD_INIT,
    CMD_UNKNOWN 
} CommandType;

// CLI functionality

CommandType stocommand(const char *command_str) {
    if (command_str == NULL) return CMD_UNKNOWN;

    if (strcmp(command_str, "ADD") == 0 || strcmp(command_str, "SET") == 0) return CMD_ADD;
    if (strcmp(command_str, "GET") == 0) return CMD_GET;
    if (strcmp(command_str, "DEL") == 0) return CMD_DEL;
    if (strcmp(command_str, "LIST-KEYS") == 0) return CMD_LIST_KEYS;
    if (strcmp(command_str, "COUNT") == 0) return CMD_COUNT;
    if (strcmp(command_str, "SAVE") == 0) return CMD_SAVE;
    if (strcmp(command_str, "LOAD") == 0) return CMD_LOAD;
    if (strcmp(command_str, "RESET") == 0) return CMD_RESET;
    if (strcmp(command_str, "HELP") == 0) return CMD_HELP;
    if (strcmp(command_str, "EXIT") == 0 || strcmp(command_str, "QUIT") == 0) return CMD_EXIT; 
    if (strcmp(command_str, "INIT") == 0) return CMD_INIT; 
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

int cmd_add(char *argument){
    char *key_str;
    char *type_str;
    char *data_str;

    #if DEBUG_MODE
        printf("[DEBUG] cmd_add: Received argument: \"%s\"\n", argument);
    #endif

    key_str = argument;
    char *first_space = strchr(key_str, ' ');
    if (first_space == NULL) {
        fprintf(stderr, "[ERROR] cmd_add: Invalid format. Expected 'KEY DATATYPE:VALUE'. Missing space after key.\n");
        return CLI_FAILURE;
    }
    *first_space = '\0'; 

    type_str = first_space + 1;
    while (*type_str && isspace((unsigned char)*type_str)) { 
        type_str++;
    }
    if (*type_str == '\0') {
        fprintf(stderr, "[ERROR] cmd_add: Invalid format. Expected 'DATATYPE:VALUE' after key. Missing type.\n");
        return CLI_FAILURE;
    }

    char *first_colon = strchr(type_str, ':');
    if (first_colon == NULL) {
        fprintf(stderr, "[ERROR] cmd_add: Invalid format. Expected 'DATATYPE:VALUE'. Missing colon after type.\n");
        return CLI_FAILURE;
    }
    *first_colon = '\0'; 

    data_str = first_colon + 1;

    #if DEBUG_MODE
        printf("[DEBUG] cmd_add: Parsed: Key='%s', Type='%s', Data='%s'\n", key_str, type_str, data_str);
    #endif

    const unsigned char *key = (const unsigned char*)key_str;
    DataTypes datatype = stodatatype(type_str);

    #if DEBUG_MODE
        printf("[DEBUG] cmd_add: Inferred type ID for '%s'.\n", type_str);
    #endif

    int error_db;

    switch (datatype) {
        case STR: { 
            error_db = db_set(key, data_str, strlen(data_str) + 1);
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: Failed to set string value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }

            break;            
        }
        
        case INT: {
            char *endptr_int;
            long data_long = strtol(data_str, &endptr_int, 10); // Use strtol for better error checking

            if (endptr_int == data_str) {
                fprintf(stderr, "[ERROR] cmd_add: Invalid integer format for '%s'. No digits found.\n", data_str);
                return CLI_FAILURE;
            }
            while (*endptr_int != '\0' && isspace((unsigned char)*endptr_int)) {
                endptr_int++;
            }
            if (*endptr_int != '\0') {
                fprintf(stderr, "[ERROR] cmd_add: Invalid characters ('%s') found after integer in '%s'.\n", endptr_int, data_str);
                return CLI_FAILURE;
            }
            if ((data_long == LONG_MAX || data_long == LONG_MIN) || data_long > INT_MAX || data_long < INT_MIN) {
                fprintf(stderr, "[ERROR] cmd_add: Integer value '%s' is out of range for type int.\n", data_str);
                return CLI_FAILURE;
            }
            
            int data_int = (int)data_long;
            error_db = db_set(key, &data_int, sizeof(int));
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: Failed to set int value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }

            break;
        }

        case HEX: {
            char *endptr_hex;
            unsigned long converted_value = strtoul(data_str, &endptr_hex, 16);

            if (converted_value == ULONG_MAX) {
                fprintf(stderr, "[ERROR] cmd_add: Hex value '%s' is too large (overflow) for unsigned long.\n", data_str);
                return CLI_FAILURE;
            }

            if (endptr_hex == data_str) {
                fprintf(stderr, "[ERROR] cmd_add: Invalid hexadecimal format for '%s'. No valid hex digits found.\n", data_str);
                return CLI_FAILURE;
            }

            char *checkptr = endptr_hex;
            while (*checkptr != '\0' && isspace((unsigned char)*checkptr)) {
                checkptr++;
            }
            if (*checkptr != '\0') {
                fprintf(stderr, "[ERROR] cmd_add: Invalid characters ('%s') found after hexadecimal number in '%s'.\n", checkptr, data_str);
                return CLI_FAILURE;
            }

            error_db = db_set(key, &converted_value, sizeof(converted_value));
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: Failed to set hex value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }
            break;  
        }

        case FILENAME: {
            error_db = db_set(key, data_str, strlen(data_str) + 1);
            if (error_db != DB_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: Failed to set filename value in database for key '%s'.\n", key_str);
                return CLI_FAILURE;
            }
            #if DEBUG_MODE
                printf("[DEBUG] cmd_add: Stored filename '%s' for key '%s'.\n", data_str, key_str);
            #endif
            break; // Added break
        }

        case DATATYPE_UNKNOWN:
        default: {
            fprintf(stderr, "[ERROR] cmd_add: Please use a known datatype annotation! And remember is case-sensitive.\n");
            return CLI_FAILURE;
        }
    }

    #if DEBUG_MODE
        printf("[DEBUG] cmd_add: Successfully processed ADD command for key '%s'.\n", key_str);
    #endif

    return CLI_SUCCESS;
}


int cmd_get(const unsigned char *argument) {

    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_get: GET command requires a non-empty key.\n");
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
        fprintf(stderr, "[ERROR] cmd_get: Key '%s' not found.\n", argument);
        return CLI_FAILURE; 
    }
}
 
int cmd_del(const unsigned char *argument) {
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_get: GET command requires a non-empty key.\n");
        return CLI_FAILURE;
    }
  
    int error = db_delete(argument);
    
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_del: An error occured while deleting the key!\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
} 
 
int cmd_list_keys(void) {

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_list_keys: Database not initialized.\n");
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
                fprintf(stderr, "[WARN] cmd_list_keys: Found node with NULL key in bucket %d.\n", i);
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

int cmd_count(void) {
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_count: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int key_count = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = hashTable[i];

        while (current != NULL) {
            if (current->key != NULL){
                key_count++;
            } else {
                fprintf(stderr, "[ERROR] cmd_count: Found node with NULL key in bucket %d.\n", i);
            }
        }
    }

    if (key_count == 0) {
        printf("[INFO] cmd_count: No keys found!\n");
    } else {
        printf("Total key found in hashTable -> %d", key_count);
    }

    return CLI_SUCCESS;
}
 
int cmd_save(const char *argument) {
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_save: SAVE command requires a non-empty filename.\n");
        return CLI_FAILURE;
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_save: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int error = db_save(argument);
    
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_save: An error occured while invoking db_save.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_load(const char *argument){
    if (argument == NULL || *argument == '\0') {
        fprintf(stderr, "[ERROR] cmd_load: LOAD command requires a non-empty filename.\n");
        return CLI_FAILURE;
    }

    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_load: Database not initialized.\n");
        return CLI_FAILURE;
    }

    int error = db_load(argument);
    
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_load: An error occured while invoking db_load.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_reset(void){
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_reset: Database not initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = destroy_db();
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_reset: An error occured while destroying the database.\n");
        return CLI_SUCCESS;
    }

    error = init_db();
    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_reset: An error occured while re-creating the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_help(void){
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
 
int cmd_exit(void){
    if (hashTable == NULL) {
        fprintf(stderr, "[ERROR] cmd_exit: Database not initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = destroy_db();

    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_exit: An error occured while destroying the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;
}

int cmd_init(void) {
    if (hashTable != NULL) {
        fprintf(stderr, "[ERROR] cmd_init: Database is already initialized.\n");
        return CLI_FAILURE;
    }
    
    int error = init_db();

    if (error != DB_SUCCESS) {
        fprintf(stderr, "[ERROR] cmd_exit: An error occured while destroying the database.\n");
        return CLI_FAILURE;
    }

    return CLI_SUCCESS;   
}

void process_command(char *line) {
    #if DEBUG_MODE
        printf("[DEBUG] process_command: Starting line parsing...\n");
    #endif

    char *saveptr;
    char *command_str = __strtok_r(line, " ", &saveptr);
    
    if (command_str == NULL) return;

    CommandType command = stocommand(command_str);
    char *command_argument = __strtok_r(NULL, "", &saveptr); 

    #if DEBUG_MODE
        printf("[DEBUG] process_command: Line parsing finisched!\n");
        printf("[DEBUG] process_command: Starting command redirection...\n");
    #endif   

    switch (command) {
        case CMD_SET:
        case CMD_ADD: {
            int error = cmd_add(command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }
        
        case CMD_GET: {
            int error = cmd_get((const unsigned char*)command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }          
        
        case CMD_DEL: {
            int error = cmd_del((const unsigned char*)command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_LIST_KEYS: {
            int error = cmd_list_keys();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_COUNT: {
            int error = cmd_count();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_SAVE: {
            int error = cmd_save(command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_LOAD: {
            int error = cmd_load(command_argument);
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_RESET: {
            int error = cmd_reset();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_HELP: {
            int error = cmd_help();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }
        
        case CMD_QUIT:
        case CMD_EXIT: {
            int error = cmd_exit();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
                return;
            }
            return;
        }

        case CMD_INIT: {
            int error = cmd_init();
            if (error != CLI_SUCCESS) {
                fprintf(stderr, "[ERROR] cmd_add: There was an error while executing the insertion of a new value.\n");
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
