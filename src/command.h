// command.h

#ifndef COMMAND_H
#define COMMAND_H

// INCLUDES

#include <stddef.h>
#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include "hashtable.h"

// MACRO

#define MAX_VALUE_SIZE 2097152
#define MAX_TOKENS 10
#define MAX_COUNT_TYPE_SIZE 32 // Consider to update this if you increase the count Instruction Set

// DATA

typedef enum : uint8_t{
    DATA_TYPE_BOOL,
    DATA_TYPE_INT,
    DATA_TYPE_STRING,
    DATA_TYPE_VOID_PTR,
    DATA_TYPE_SIZE,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_HASHTABLE_T_PTR,
    DATA_TYPE_BIN,
    DATA_TYPE_DATA_ENTRY_PTR,
    DATA_TYPE_EMPTY,
    DATA_TYPE_ERROR
} data_type_t;

typedef enum : uint8_t{
    CMD_TYPE_CREATE,
    CMD_TYPE_GET,
    CMD_TYPE_SET,
    CMD_TYPE_ADD,
    CMD_TYPE_DESTROY,
    CMD_TYPE_DEL,
    CMD_TYPE_EXIST,
    CMD_TYPE_REPLACE,
    CMD_TYPE_RESIZE,
    CMD_TYPE_CLEAR,
    CMD_TYPE_LOADFACTOR,
    CMD_TYPE_COUNT,
    CMD_TYPE_ERROR,
    CMD_TYPE_EMPTY
} cmd_function_type;

typedef enum : uint8_t{
    CMD_COUNT_CAPACITY,
    CMD_COUNT_MEMORY_USAGE,
    CMD_COUNT_TOTAL_ELEM,
    CMD_COUNT_OCCUPIED_BUCKET
} cmd_count_t;

typedef struct data_entry_t{ // DB stored structure
    size_t size;
    data_type_t type;
    unsigned char data[];
} data_entry_t;


typedef struct command_data_t{ // Command necessary input
    cmd_function_type tag;        
    union in{
        struct create_input{
            size_t db_size;
        }create_input;       

        struct get_input{
            const unsigned char* key;
        }get_input;

        struct set_input{
            const unsigned char* key;
            data_entry_t* value;
        }set_input;        

        struct add_input{
            const unsigned char* key;
            data_entry_t* value;
        }add_input;       

        struct destroy_input{
            char _dummy;
        }destroy_input;        

        struct del_input{
            const unsigned char* key;
        }del_input;    

        struct exist_input{
            const unsigned char* key;
        }exist_input;

        struct replace_input{
            const unsigned char* key;
            data_type_t* new_value;
        }replace_input;

        struct resize_input{
            size_t new_size;
        }resize_input;

        struct clear_input{
            char _dummy;
        }clear_input;

        struct load_factor_input{
            char _dummy;
        }load_factor_input;

        struct count_input{
            cmd_count_t type;
        }count_input;
    }in;
}command_data_t;


typedef struct command_result_t{
    cmd_function_type type;
    union out{
        struct create_output{
            hashtable_t* hashtable;
        }create_output;

        struct get_output{
            data_entry_t* value; 
        }get_output;

        struct set_output{
            int error;
        }set_output; 

        struct add_output{
            int error;
        }add_output;

        struct destroy_output{
            int error;
        }destroy_output;

        struct del_output{
            int error;
        }del_output;

        struct exist_output{
            bool existence;
        }exist_output;

        struct replace_output{
            int error;
        }replace_output;

        struct resize_output{
            int error;
        }resize_output;

        struct clear_output{
            int error;
        }clear_output;

        struct load_factor_output{
            double load_factor;
        }load_factor_output;

        struct count_output{
            cmd_count_t type;
            data_type_t out_type;
            union count_t{
                double counter_d;
                size_t counter_s;
            }count_t;
        }count_output;
    }output;
} command_result_t;

typedef command_result_t (*command_proc)(hashtable_t* context, command_data_t* data);

typedef struct command{
    const char* name;
    cmd_function_type tag;
    command_proc proc;
    int arity;
    const char* flags;
} command;

typedef struct execute_result_t{
    int status_code;

    unsigned char* body;
    size_t body_length;

    data_type_t content_type;
} execute_result_t;

typedef struct command_registry command_registry;

// PUBLIC API

    // EXECUTOR
    execute_result_t execute_command(command_registry* reg, hashtable_t* context,
                                     const char* command_name, int argc, char* argv[],
                                     const size_t arg_lengths[]);



    command_registry* registry_create();
    int registry_destroy(command_registry** reg);
    ssize_t find_command_index(command_registry* reg, const char* command_name);
    bool stocmdcount(const char* subtype_str, cmd_count_t* out_type);
    void free_execute_result(execute_result_t* result);
    void destroy_data_entry(void* value);

#endif
