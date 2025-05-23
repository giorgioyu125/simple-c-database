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
    
    #define TABLE_SIZE 4096 // Update this to globabl constant of init function

// Data

typedef enum {
    STR,
    INT,
    HEX,
    FILENAME,
    DATATYPE_UNKNOWN
} DataTypes;


typedef enum {
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
    CMD_INIT,
    CMD_UNKNOWN 
} CommandType;

// public API (function prototypes)

    // Helper functions
    CommandType stocommand(const char* command_str, const char* db_type);
    DataTypes stodatatype(const char* datatype_str);
    int process_command(char* line, char* db_type, size_t db_size);

    // Set
    int cmd_set_add(char* argument);
    int cmd_set_exist(char* argument, bool* existence);
    int cmd_set_del(char* argument);
    int cmd_set_list_keys(const char* db_type);
    int cmd_set_count(char* db_type, size_t* counter_ptr);
    int cmd_set_save(char* argument);
    int cmd_set_load(char* argument);
    int cmd_set_reset(char* argument, char* db_type, size_t db_size);
    int cmd_set_exit(void);
    int cmd_set_help(void);

    // Table
    int cmd_table_add(char *argument);
    int cmd_table_get(const unsigned char *argument);
    int cmd_table_del(const unsigned char *argument);
    int cmd_table_list_keys(size_t db_size);
    int cmd_table_count(size_t db_size);
    int cmd_table_save(const char *argument);
    int cmd_table_load(const char *argument);
    int cmd_table_reset(char* db_type, size_t db_size);
    int cmd_help(void);
    int cmd_exit(void);
    int cmd_init(char* command_argument, size_t db_size, char* db_type);

#endif
