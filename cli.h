// Include

#ifndef CLI_H
#define CLI_H

#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"
#include "hashset.h"
#include "hashing_functionality.h"
#include "string_functionality.h"

// Defines
    
    // OS

    #define MAX_FILENAME_LENGHT 260

    // Debug Mode

    #define DEGUB_MODE 1

    // Error handling

    #define CLI_SUCCESS 0
    #define CLI_FAILURE -1

    // Table/Set
    
    #define TABLE_SIZE 4096

// Data

typedef enum {
    STR,
    INT,
    HEX,
    FILENAME,
    DATATYPE_UNKNOWN
} DataTypes;

typedef enum {
    SET,
    TABLE
} DataBase_Type;

typedef enum {
    // SET
    CMD_SET_ADD,
    CMD_SET_SET,
    CMD_SET_GET,
    CMD_SET_DEL,
    CMD_SET_LIST_KEYS,
    CMD_SET_COUNT,
    CMD_SET_SAVE,
    CMD_SET_LOAD,
    CMD_SET_HELP,
    CMD_SET_RESET,
    CMD_SET_EXIT,
    CMD_SET_QUIT,
    CMD_SET_INIT,

    // TABLE
    CMD_TABLE_ADD,
    CMD_TABLE_SET,
    CMD_TABLE_GET,
    CMD_TABLE_DEL,
    CMD_TABLE_LIST_KEYS,
    CMD_TABLE_COUNT,
    CMD_TABLE_SAVE,
    CMD_TABLE_LOAD,
    CMD_TABLE_RESET,
    CMD_TABLE_HELP,
    CMD_TABLE_EXIT,
    CMD_TABLE_QUIT,
    CMD_TABLE_INIT,

    // GENERIC
    CMD_UNKNOWN 
} CommandType;

// Public API (function prototypes)

    // Helper functions
    CommandType stocommand(const char* command_str, char* db_type);
    DataTypes stodatatype(const char* datatype_str);
    int process_command(char *line, char* db_type, size_t db_size, void* out_db);
    int update_db_type(char* command_argument, char* command_str, char** db_type_ptr, size_t* db_size_ptr);

    // Set
    int cmd_set_add(char* argument, HashSet* hashset);
    int cmd_set_exist(char* argument, bool* existence, HashSet* hashset);
    int cmd_set_del(char* argument, HashSet* hashset);
    int cmd_set_list_keys(HashSet* hashset);
    int cmd_set_count(HashSet* hashset);
    int cmd_set_save(char* argument, HashSet* hashset);
    int cmd_set_load(char* argument, HashSet* hashset);
    int cmd_set_reset(char* command_argument, size_t db_size, HashSet* hashset);
    int cmd_set_exit(HashSet* hashset);
    int cmd_set_help(void);
    int cmd_set_init(char* command_argument, size_t db_size, HashSet* out_hashset);


    // Table
    int cmd_table_add(char *argument, Node** hashTable);
    int cmd_table_get(const unsigned char *argument, Node** hashTable);
    int cmd_table_del(const unsigned char *argument, Node** hashTable);
    int cmd_table_list_keys(size_t db_size, Node** hashTable);
    int cmd_table_count(size_t db_size, Node** hashTable);
    int cmd_table_save(const char *argument, Node** hashTable);
    int cmd_table_load(const char *argument, size_t db_size, Node** hashTable);
    int cmd_table_reset(char* command_argument, size_t db_size, Node** hashTable);
    int cmd_help(Node** hashTable);
    int cmd_exit(Node** hashTable);
    int cmd_table_init(char* command_argument, size_t db_size, Node** hashTable);

#endif
