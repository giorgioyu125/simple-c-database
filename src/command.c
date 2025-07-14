// command.c

// HEADER

#include "command.h"
#include "hashtable.h"
#include "string_functionality.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


// PRIVATE API

static int compare_command_names(const void* key, const void* element);

static command_result_t cmd_get(hashtable_t* context, command_data_t* input);
static command_result_t cmd_set(hashtable_t* context, command_data_t* input);
static command_result_t cmd_add(hashtable_t* context, command_data_t* input);
static command_result_t cmd_del(hashtable_t* context, command_data_t* input);
static command_result_t cmd_exist(hashtable_t* context, command_data_t* input);
static command_result_t cmd_replace(hashtable_t* context, command_data_t* input);
static command_result_t cmd_resize(hashtable_t* context, command_data_t* input);
static command_result_t cmd_clear(hashtable_t* context, command_data_t* input);
static command_result_t cmd_load_factor(hashtable_t* context, command_data_t* input);
static command_result_t cmd_count(hashtable_t* context, command_data_t* input);

static int build_command_data(cmd_function_type tag, char* argv[], const size_t args_lengths[], command_data_t* out_data);

static execute_result_t create_error_response(int status, const char* message);



void free_execute_result(execute_result_t* result){
    if (result == NULL){
        return;
    }

    free(result->body);

    result->body = NULL;
    result->body_length = 0; 
}

size_t std_value_sizer(const void* value){
    if (value == NULL){
        return 0;
    }

    data_entry_t* entry = (data_entry_t*)value;

    return sizeof(data_entry_t) + entry->size;
}

    // Value Destroyers

void std_value_destroy(data_entry_t* value){
    if (value == NULL){
        return;
    }

    free(value);
    return;
}

void destroy_value_wrapper(void* data) {
    std_value_destroy((data_entry_t*)data);
}

struct command_registry{
    struct command* commands;
    size_t count;
};

int registry_destroy(command_registry** reg){
    if (reg == NULL || *reg == NULL){
        return -1;
    }
    
    (*reg)->count = 0;
    (*reg)->commands = NULL;

    free(*reg);
    *reg = NULL;

    return 0;
}

static int compare_command_names(const void* key, const void* element){
    const char* command_name = (const char*)key;
    const command* cmd_element = (const command*)element;
    return strcmp(command_name, cmd_element->name);
}

static ssize_t find_command_index(command_registry* reg, const char* command_name){
    if ((reg == NULL) || (command_name == NULL)) {
        return -1; 
    }

    const command* found_command = bsearch(command_name, reg->commands, reg->count, sizeof(command), compare_command_names);

    if (found_command == NULL) {
        return -1; 
    }

    return (ssize_t)(found_command - reg->commands);
}

    // CMD FUNCTIONS

static command_result_t cmd_get(hashtable_t* context, command_data_t* input){
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL)){
        return result;
    }

    const unsigned char* key = input->in.get_input.key;

    if (is_key_valid(key) == false){
        return result;
    }
    
    fprintf(stderr, "[INFO] cmd_get: Executing GET for key: '%s'.\n", key);
    void* generic_ptr = table_get(context, key, std_value_sizer);

    if (generic_ptr == NULL){
        result.type = CMD_TYPE_EMPTY;
        return result;
    }

    result.type = CMD_TYPE_GET;
    result.output.get_output.value = (data_entry_t*)generic_ptr;

    return result;
}

static command_result_t cmd_set(hashtable_t* context, command_data_t* input) {
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL)){
        return result;
    }

    const unsigned char* key = input->in.set_input.key;
    data_entry_t* value = (data_entry_t*)input->in.set_input.value;

    if ((is_key_valid(key) == false) || (value == NULL)){
        return result;
    }

    int error = table_set(context, key, value, destroy_value_wrapper);

    result.type = CMD_TYPE_SET;
    result.output.set_output.error = error;
    return result;
}

static command_result_t cmd_add(hashtable_t* context, command_data_t* input){
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL)){
        return result;
    }

    const unsigned char* key = input->in.set_input.key;
    data_entry_t* value = (data_entry_t*)input->in.set_input.value;

    if ((is_key_valid(key) == false) || (value == NULL)){
        return result;
    }

    int error = table_add(context, key, value);

    result.type = CMD_TYPE_ADD;
    result.output.add_output.error = error;
    return result;
}

static command_result_t cmd_del(hashtable_t* context, command_data_t* input) {
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL) || (is_key_valid(input->in.del_input.key) == false)) {
        return result;
    }

    const unsigned char* key_to_delete = input->in.del_input.key;
    int error = table_delete(context, key_to_delete, destroy_value_wrapper);

    result.output.del_output.error = error;

    if (error == 0) {
        fprintf(stderr, "[INFO] cmd_del: Successfully executed DEL for key: '%s'.\n", key_to_delete);
        result.type = CMD_TYPE_DEL;
    } else {

        fprintf(stderr, "[ERROR] cmd_del: Failed to delete key '%s' (error code: %d).\n", key_to_delete, error);
    }

    return result;
}

static command_result_t cmd_exist(hashtable_t* context, command_data_t* input) {
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL) || (is_key_valid(input->in.exist_input.key) == false)) {
        return result;
    }

    const unsigned char* actual_key = input->in.exist_input.key;

    fprintf(stderr, "[INFO] cmd_exist: Executing EXIST for key: '%s'.\n", actual_key);

    bool existence = table_exist(context, actual_key);

    result.output.exist_output.existence = existence;
    result.type = CMD_TYPE_EXIST;

    return result;
}

static command_result_t cmd_replace(hashtable_t* context, command_data_t* input){
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL) || (is_key_valid(input->in.replace_input.key) == false) || 
        (input->in.replace_input.new_value == NULL)){
        return result;
    }

    const unsigned char* key = input->in.replace_input.key;
    data_entry_t* new_value = (data_entry_t*)input->in.replace_input.new_value;

    fprintf(stderr, "[INFO] cmd_replace: Attempting to REPLACE value for key: '%s'.\n", key);

    int error_code = table_replace(context, key, new_value, destroy_value_wrapper);

    if (error_code != 0){
        fprintf(stderr, "[ERROR] cmd_replace: Failed to replace key '%s' (error code: %d). Key might not exist.\n", key, error_code);
        return result;
    }

    result.type = CMD_TYPE_REPLACE;
    result.output.replace_output.error = 0; 

    return result;
}

static command_result_t cmd_resize(hashtable_t* context, command_data_t* input){
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL) || (input->in.resize_input.new_size == 0)){
        return result;
    }

    size_t new_size = input->in.resize_input.new_size;

    fprintf(stderr, "[INFO] cmd_resize: Attempting to resize table to %zu buckets.\n", new_size);

    int error_code = table_resize(context, new_size);

    if (error_code != 0) {
        fprintf(stderr, "[ERROR] cmd_resize: Failed to resize table (error code: %d).\n", error_code);
        return result;
    }

    result.type = CMD_TYPE_RESIZE;
    result.output.resize_output.error = 0; 

    return result;
}

static command_result_t cmd_clear(hashtable_t* context, command_data_t* input){
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL)){
        return result;
    }   

    fprintf(stderr, "[INFO] cmd_clear: Executing CLEAR for context: '%p'.\n", (void*)context);

    int error = table_clear(context, destroy_value_wrapper);
    
    result.type = CMD_TYPE_CLEAR;
    result.output.clear_output.error = error;

    return result;
}

static command_result_t cmd_load_factor(hashtable_t* context, command_data_t* input){
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL)) {
        return result;
    }

    fprintf(stderr, "[INFO] cmd_load_factor: Calculating load factor for context: '%p'.\n", (void*)context);

    double load_factor = table_load_factor(context);

    result.type = CMD_TYPE_LOADFACTOR;
    result.output.load_factor_output.load_factor = load_factor;

    return result;
}

static command_result_t cmd_count(hashtable_t* context, command_data_t* input) {
    command_result_t result = {0};
    result.type = CMD_TYPE_ERROR;
    if ((context == NULL) || (input == NULL)) {
        return result;
    }

    const cmd_count_t count_type = input->in.count_input.type;
    switch (count_type){
        case CMD_COUNT_OCCUPIED_BUCKET:{
            double occupied = table_occupied_bucket_counter(context);
            result.output.count_output.count_t.counter_d = occupied;
            break;
        }

        case CMD_COUNT_CAPACITY:{
            size_t capacity = table_capacity(context);
            result.output.count_output.count_t.counter_s = capacity;
            break;
        }

        case CMD_COUNT_MEMORY_USAGE:{
            size_t memory = table_memory_usage(context, std_value_sizer);
            result.output.count_output.count_t.counter_s = memory;
            break;
        }

        case CMD_COUNT_TOTAL_ELEM:{
            size_t total = table_total_elem(context);
            result.output.count_output.count_t.counter_s = total;
            break;
        }

        default: {
            fprintf(stderr, "[ERROR] cmd_count: Unknown or unsupported COUNT subtype enum value: %d.\n", count_type);
            return result;
        }
    }

    result.type = CMD_TYPE_COUNT;
    return result;
}

// Command Table

static command command_table[] = { // Name needs to be in lexicographic order
    // name         tag                     proc               arity   flags
    //----------------------------------------------------------------------
    { "ADD",        CMD_TYPE_ADD,           cmd_add,           2,      "wa"},
    { "CLEAR",      CMD_TYPE_CLEAR,         cmd_clear,         0,      "w" },
    { "COUNT",      CMD_TYPE_COUNT,         cmd_count,         1,      "r" },
    { "DEL",        CMD_TYPE_DEL,           cmd_del,           1,      "w" },
    { "EXIST",      CMD_TYPE_EXIST,         cmd_exist,         1,      "r" },
    { "GET",        CMD_TYPE_GET,           cmd_get,           1,      "r" },
    { "LOADFACTOR", CMD_TYPE_LOADFACTOR,    cmd_load_factor,   0,      "r" },
    { "REPLACE",    CMD_TYPE_REPLACE,       cmd_replace,       2,      "wa"},
    { "RESIZE",     CMD_TYPE_RESIZE,        cmd_resize,        1,      "w" },
    { "SET",        CMD_TYPE_SET,           cmd_set,           2,      "wa"},

    { NULL,         255,                    NULL,              0,      NULL}
};

// Execute Helper function

bool stocmdcount(const char* subtype_str, cmd_count_t* out_type) {
    if (subtype_str == NULL || subtype_str[0] == '\0' || out_type == NULL) {
        return false;
    }

    switch (toupper((unsigned char)subtype_str[0])) {
        case 'C':
            if (strcmp(subtype_str, "CAPACITY") == 0) {
                *out_type = CMD_COUNT_CAPACITY;
                return true;
            }
            break;

        case 'M':
            if (strcmp(subtype_str, "MEMORY_USAGE") == 0) {
                *out_type = CMD_COUNT_MEMORY_USAGE;
                return true;
            }
            break;

        case 'T':
            if (strcmp(subtype_str, "TOTAL_ELEM") == 0) {
                *out_type = CMD_COUNT_TOTAL_ELEM;
                return true;
            }
            break;

        case 'O':
            if (strcmp(subtype_str, "OCCUPIED_BUCKET") == 0) {
                *out_type = CMD_COUNT_OCCUPIED_BUCKET;
                return true;
            }
            break;
    }

    return false;
}

command_registry* registry_create(){
    command_registry* reg = malloc(sizeof(struct command_registry));
    if (reg == NULL) {
        fprintf(stderr, "[ERROR] registry_create: Failed to allocate memory for command registry.\n");
        return NULL;
    }

    size_t command_count = (sizeof(command_table) / sizeof(command)) - 1;

    reg->commands = command_table;
    reg->count = command_count;

    printf("[INFO] registry_create: Command registry created with %zu commands.\n", reg->count);

    return reg;
}

static int build_command_data(cmd_function_type tag, char* argv[], const size_t args_lengths[], command_data_t* out_data){
    out_data->tag = tag;

    switch (tag){
        case CMD_TYPE_REPLACE:
        case CMD_TYPE_SET:
        case CMD_TYPE_ADD:{
            const unsigned char* key = (const unsigned char*)argv[0];
            if (is_key_valid(key) == false) {
                fprintf(stderr, "[ERROR] build_command_data: Provided key is not valid.\n");
                return -1;
            }
            
            data_entry_t* value = malloc(sizeof(data_entry_t) + args_lengths[1]);
            if (value == NULL) {
                fprintf(stderr, "[ERROR] build_command_data: Memory allocation for value failed.\n");
                return -1;
            }
            
            value->size = args_lengths[1];
            memcpy(value->data, argv[1], value->size);

            
            out_data->in.set_input.key = key;
            out_data->in.set_input.value = value;
            break;
        }

        case CMD_TYPE_GET:
        case CMD_TYPE_DEL:
        case CMD_TYPE_EXIST:{
            const unsigned char* key = (const unsigned char*)argv[0];
            if (is_key_valid(key) == false){
                fprintf(stderr, "[ERROR] build_command_data: Provided key is not valid.\n");
                return -1;
            }
            out_data->in.get_input.key = key;
            break;
        }


        case CMD_TYPE_RESIZE:{
            char* endptr;
            unsigned long new_size = strtoul(argv[0], &endptr, 10);
            if (endptr == argv[0] || *endptr != '\0'){
                fprintf(stderr, "[ERROR] build_command_data: Provided size for RESIZE is not a valid number: '%s'.\n", argv[0]);
                return -1;
            }
            out_data->in.resize_input.new_size = (size_t)new_size;
            break;
        }

        case CMD_TYPE_CLEAR:
        case CMD_TYPE_LOADFACTOR:{
            out_data->in.load_factor_input._dummy = 0;
            break;
        }

        case CMD_TYPE_COUNT:{
            cmd_count_t count_type_enum; 
            if (stocmdcount(argv[0], &count_type_enum) == false) {
                fprintf(stderr, "[ERROR] build_command_data: Provided COUNT subtype is not valid: '%s'.\n", argv[0]);
                return -1;
            }
            out_data->in.count_input.type = count_type_enum;
            break;
        }

        case CMD_TYPE_ERROR:
        case CMD_TYPE_EMPTY:
        default:{
            fprintf(stderr, "[ERROR] build_command_data: Received an unexpected or unsupported command tag: %d.\n", tag);
            return -1;
        }
    }

    return 0;
}

static execute_result_t create_error_response(int status, const char* message) {
    unsigned char* body = ustrdup(message);
    if (body == NULL) {
        return (execute_result_t){ 
            .status_code = 500, 
            .body = NULL, 
            .body_length = 0,
        };
    }

    return (execute_result_t){
        .status_code = status,
        .body = body, 
        .body_length = ustrlen(body),
    };
}

// PUBLIC API

execute_result_t execute_command(server_context_t* server_ctx,
                                 const char* command_name, int argc, char* argv[],
                                 const size_t args_lengths[])
{
    command_registry* reg = server_ctx->reg;
    hashtable_t* context = server_ctx->db;

    if ((reg == NULL) || (command_name == NULL)) {
        return create_error_response(400, "Invalid arguments to dispatcher");
    }

    ssize_t command_index = find_command_index(reg, command_name);
    if (command_index == -1){
        return create_error_response(404, "Command not found");
    }

    const command* cmd = &(reg->commands[command_index]);
    if (cmd->arity != argc) {
        return create_error_response(400, "Incorrect number of arguments");
    }

    command_data_t command_inputs = {0};
    if (build_command_data(cmd->tag, argv, args_lengths, &command_inputs) != 0){
        return create_error_response(400, "Invalid argument format");
    }
    
    command_result_t cmd_result = cmd->proc(context, &command_inputs);
    
    if (cmd_result.output.set_output.error != 0) {
        if (strchr(cmd->flags, 'a') != 0) {
            destroy_value_wrapper(command_inputs.in.set_input.value->data);
        }
        return create_error_response(409, TCP_OPERATION_FAILED);
    }

    execute_result_t final_result = { .status_code = 200 };
    switch (cmd_result.type){
        case CMD_TYPE_GET:{ 
            final_result.body = cmd_result.output.get_output.value->data;
            final_result.body_length = cmd_result.output.get_output.value->size;
            break;
        }

        case CMD_TYPE_SET:
        case CMD_TYPE_ADD:
        case CMD_TYPE_DEL:
        case CMD_TYPE_REPLACE:
        case CMD_TYPE_RESIZE:
        case CMD_TYPE_CLEAR:{
            if (cmd_result.output.set_output.error != 0){
                return create_error_response(409, TCP_OPERATION_FAILED);
            } else{
                final_result.body = ustrdup(TCP_SUCCESS); 
                final_result.body_length = 4;
            }
            break;
        }

        case CMD_TYPE_EXIST:{
            final_result.body = ustrdup(cmd_result.output.exist_output.existence ? "1" : "0");
            if (final_result.body == NULL){
                return create_error_response(500, TCP_MEMORY_ERROR);
            }

            final_result.body_length = 2;
            break;
        }

        case CMD_TYPE_COUNT: {
            int required_len = 0; 
            char temp_buffer[64];

            switch (cmd_result.output.count_output.type) {
                case CMD_COUNT_OCCUPIED_BUCKET: {
                    double value = cmd_result.output.count_output.count_t.counter_d;

                    required_len = snprintf(temp_buffer, sizeof(temp_buffer), "%.2f", value);
                    break;
                }
                case CMD_COUNT_CAPACITY:
                case CMD_COUNT_MEMORY_USAGE:
                case CMD_COUNT_TOTAL_ELEM: {
                    size_t value = cmd_result.output.count_output.count_t.counter_s;

                    required_len = snprintf(temp_buffer, sizeof(temp_buffer), "%zu", value);
                    break;
                }
                default:
                    return create_error_response(500, "Unknown COUNT result type");
            }

            if (required_len < 0) {
                return create_error_response(500, "Failed to format COUNT result");
            }

            final_result.body = malloc((size_t)required_len + 1);
            if (final_result.body == NULL) {
                return create_error_response(500, TCP_MEMORY_ERROR);
            }

            memcpy(final_result.body, temp_buffer, (size_t)required_len);
            final_result.body[required_len] = '\0'; 

            final_result.body_length = (size_t)required_len;

            break;
        }

        case CMD_TYPE_LOADFACTOR:{
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.4f", cmd_result.output.load_factor_output.load_factor);

            final_result.body = ustrdup(buffer);
            if (final_result.body == NULL) return create_error_response(500, TCP_MEMORY_ERROR);
            final_result.body_length = ustrlen(final_result.body);
            break;
        }

        case CMD_TYPE_EMPTY:
            return create_error_response(404, TCP_KEY_NOT_FOUND);
        case CMD_TYPE_ERROR:
            return create_error_response(500, TCP_INTERNAL_ERROR);
        default:
            return create_error_response(500, TCP_NON_DEFAULT_T);
    }

    return final_result;
}
