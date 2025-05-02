// Includes

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "key-value_db.h"

// Error handling

#define CLI_SUCCESS 0
#define CLI_ERROR -1

// Data

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

int cmd_add

static void process_command(char *line) {
    char *saveptr;
    char *command_str = strtok_r(line, " ", &saveptr);
    
    if (command_str == NULL) return;

    CommandType command = stocommand(command_str);
    char *command_argument = strtok_r(NULL, "", &saveptr); 

    

    switch (command) {
        case CMD_ADD: {
            

        }
        
        case CMD_GET: {


        }

        case CMD_DEL: {

        }

        case CMD_LIST_KEYS: {

        }

        case CMD_COUNT: {

        }

        case CMD_SAVE: {

        }

        case CMD_LOAD: {

        }

        case CMD_RESET: {

        }

        case CMD_HELP: {

        }

        case CMD_EXIT: {

        }

        case CMD_INIT: {

        }

        case CMD_UNKNOWN: {

        }
    }
}




static void print_help(void) {
    printf("\n--- SimpleDB Command Help ---\n");
    printf("Commands are case-sensitive.\n\n");

    // Core Commands
    printf("  HELP              Show this help message\n");
    printf("  EXIT / QUIT       Exit the application\n\n");

    // Database Operations
    printf("  ADD <key> <value>   Add or update a key with a given value.\n");
    printf("                      Value representation examples:\n");
    printf("                      ADD mykey Some text value\n");
    printf("                      ADD data_hex hex:0123ABCD          (Use hex: prefix for hex data)\n");
    printf("                      ADD picture file:/path/to/image.jpg (Use file: prefix to load from file)\n");
    printf("  GET <key>           Retrieve and display the value for the specified key.\n");
    printf("  DEL <key>           Delete the key-value pair associated with the key.\n");
    printf("  LIST-KEYS           List all keys currently stored in the database.\n");
    printf("  COUNT               Display the total number of key-value pairs stored.\n");
    printf("  COUNT <index>       (Optional) Display items in a specific hash bucket.\n"); 
    printf("\n");

    // Database Management
    printf("  SAVE <filename>   Save the current in-memory database state to the specified file.\n");
    printf("  LOAD <filename>   Load database state from the specified file (clears current data first!).\n");
    printf("  RESET             Clear all data from the current in-memory database (requires confirmation).\n");
    printf("\n--- End Help ---\n");
}

void run_cli(void) {
    return;
}

int main(void) {
    print_help();
    return 0;
}
