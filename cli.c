// Includes

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
    char *end_key = strchr(argument, ' ');
    char *end_type = strchr(argument, ':');
    
    if ((end_key == NULL) || (end_type == NULL)) {
        fprintf(stderr, "[ERROR] cmd_add: the provided command have a wrong format, read the help!\n");
        return CLI_FAILURE;
    }

    #if DEGUB_MODE
        printf("[DEBUG] cmd_add: Starting command argument processing...\n");
    #endif

    *end_key = '\0';
    const unsigned char *key = (const unsigned char*)argument;
    
    *end_type = '\0';
    char *str_type = end_key + 1;

    const char *data = end_type + 1;

    int datatype = stodatatype(str_type);

    #if DEBUG_MODE
        printf("[DEBUG] cmd_add: String parsed successfully, infered type -> '%s'.\n", str_type);
    #endif

    switch (datatype) {
        case STR: { 
            int error = db_set(key, data, sizeof(data));
            if (error != DB_SUCCESS) {
                return CLI_FAILURE;
            }
        }
        
        case INT: {

            int data_int = atoi(data);
            int error = db_set(key, &data_int, sizeof(int));
            if (error != DB_SUCCESS) {
                return CLI_FAILURE;
            }
        }

        case HEX: {
            char *endptr; 
            unsigned long converted_value;

            converted_value = strtoul(data, &endptr, 16);

            if ((converted_value == LONG_MAX) || (converted_value == LONG_MIN)) {
                fprintf(stderr, "[ERROR] cmd_add: The value '%s' is outside the range for unsigned long.\n", data);
                return CLI_FAILURE;
            }

            if (endptr == data) {
                fprintf(stderr, "[ERROR] cmd_add: Unvalid hexidecimal format for '%s'.\n", data);
                return CLI_FAILURE;
            }

            char *checkptr = endptr;

            while (*checkptr != '\0' && isspace((unsigned char)*checkptr)) {
                checkptr++;
            }

            if (*checkptr != '\0') {
                fprintf(stderr, "[ERROR] cmd_add: Unvalid characters ('%s') found in the hexidecimal number -> '%s'.\n", endptr, data);
                return CLI_FAILURE;
            }

            endptr = NULL;
            checkptr = NULL;

            int error = db_set(key, &converted_value, sizeof(converted_value));
            if (error != DB_SUCCESS) {
                return CLI_FAILURE;
            }

                        
            return CLI_SUCCESS;
        }

        case FILENAME: {
            FILE *file_ptr = fopen(data, "r");
            
            if (file_ptr == NULL) {
                fprintf(stderr, "[ERROR] cmd_add: '%s' doesnt'exist or is not readable.\n", data);
                return CLI_FAILURE;
            }
            
            int error = db_set(key, file_ptr, sizeof(*file_ptr));
            if (error != DB_SUCCESS) {
                return CLI_FAILURE;
            }
            
            #if DEGUB_MODE
                printf("[DEBUG] cmd_add: ")
            #endif
        }
    }

    int error = 0;

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
    #if DEGUB_MODE
        printf("[DEBUG] process_command: Starting line parsing...\n");
    #endif

    char *saveptr;
    char *command_str = strtok_r(line, " ", &saveptr);
    
    if (command_str == NULL) return;

    CommandType command = stocommand(command_str);
    char *command_argument = strtok_r(NULL, "", &saveptr); 

    #if DEGUB_MODE
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

    #if DEGUB_MODE
        printf("[DEBUG] process_command: Command redirection finished!\n");
    #endif
}
