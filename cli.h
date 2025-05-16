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

    // Debug Mode

    #define DEGUB_MODE 1

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

    // Table

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
    CMD_SET_ADD,
    CMD_SET_SET,
    CMD_SET_GET,
    CMD_SET_DEL,
    CMD_SET_LIST_KEYS,
    CMD_SET_COUNT,
    CMD_SET_SAVE,
    CMD_SET_LOAD,
    CMD_SET_RESET,
    CMD_SET_HELP,
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

CommandType stocommand(const char* command_str, const char* db_type);
DataTypes stodatatype(const char *datatype_str);
int cmd_table_add(char *argument);
int cmd_table_get(const unsigned char *argument);
int cmd_table_del(const unsigned char *argument);
int cmd_table_list_keys(void);
int cmd_table_count(void);
int cmd_table_save(const char *argument);
int cmd_table_load(const char *argument);
int cmd_table_reset(void);
int cmd_help(void);
int cmd_exit(void);
int cmd_init(const unsigned char* db_type);
void process_command(char *line);

#endif
