// HEADER

#include "command.h"
#include "hashtable.h"
#include "string_functionality.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// PRIVATE API

size_t std_value_sizer(const void* value) {
    if (value == NULL) {
        return 0;
    }

    data_entry_t* entry = (data_entry_t*)value;

    return sizeof(data_entry_t) + entry->size;
}

void destroy_data_entry(void* value) {
    if (value == NULL) {
        return;
    }

    free(value);
    return;
}

struct command_registry {
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

static int compare_command_names(const void* key, const void* element) {
    const char* command_name = (const char*)key;
    const command* cmd_element = (const command*)element;
    return strcmp(command_name, cmd_element->name);
}

ssize_t find_command_index(command_registry* reg, const char* command_name) {
    if ((reg == NULL) || (command_name == NULL)) {
        return -1; 
    }

    const command* found_command = bsearch(command_name, reg->commands, reg->count, sizeof(command), compare_command_names);

    if (found_command == NULL) {
        return -1; 
    }

    return (ssize_t)(found_command - reg->commands);
}

// CMD functions

static command_result_t cmd_get(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL)){
        return result;
    }

    const unsigned char* key = input->in.get_input.key;

    if (is_str_valid(key) == false){
        return result;
    }
    
    fprintf(stderr, "[INFO] cmd_get: Executing GET for key: '%s'.\n", key);
    void* generic_ptr = table_get(context, key);

    if (generic_ptr == NULL){
        result.type = CMD_TYPE_EMPTY;
        return result;
    }

    result.type = CMD_TYPE_GET;
    result.output.get_output.value = (data_entry_t*)generic_ptr;

    return result;
}

static command_result_t cmd_set(hashtable_t* context, command_data_t* input) {
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL)){
        return result;
    }

    const unsigned char* key = input->in.set_input.key;
    data_entry_t* value = (data_entry_t*)input->in.set_input.value; // !! IMPORTANT

    if ((key == NULL) || (is_str_valid(key) == false) || (value == NULL)){
        return result;
    }

    int error = table_set(context, key, value, destroy_data_entry);

    if (error != 0){
        return result;
    }
    
    result.type = CMD_TYPE_SET;
    result.output.set_output.error = error;
    return result;
}

static command_result_t cmd_add(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL)){
        return result;
    }

    const unsigned char* key = input->in.set_input.key;
    data_entry_t* value = (data_entry_t*)input->in.set_input.value; // !! IMPORTANT

    if ((key == NULL) || (is_str_valid(key) == false) || (value == NULL)){
        return result;
    }

    int error = table_add(context, key, value);

    if (error != 0){
        return result;
    }
    
    result.type = CMD_TYPE_ADD;
    result.output.add_output.error = error;
    return result;
}

static command_result_t cmd_create(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR , .output = {0} };
    if ((input == NULL) || (input->in.create_input.db_size == 0) || (context != NULL)){
        return result;
    }

    printf("[INFO] cmd_create: Executing CREATE for size: '%zu'.\n", input->in.create_input.db_size);

    hashtable_t* new_context = table_create(input->in.create_input.db_size);
    if (new_context == NULL){
        result.type = CMD_TYPE_ERROR;
        return result;
    }

    result.type = CMD_TYPE_CREATE;
    result.output.create_output.hashtable = new_context;
        
    return result;
}

static command_result_t cmd_destroy(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL)){
        return result;
    }
    
    printf("[INFO] cmd_destroy: Executing DESTROY on context '%p'.\n", (void*)context);

    int error = table_destroy(context, destroy_data_entry);
    if (error != 0){
        result.type = CMD_TYPE_ERROR;
        return result;
    }

    result.output.destroy_output.error = error;
    result.type = CMD_TYPE_DESTROY;
    return result;
}

static command_result_t cmd_del(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL) || (input->in.del_input.key == NULL)){
        return result;
    }

    int error = table_delete(context, input->in.del_input.key, destroy_data_entry);
    if (error != 0){
        return result;
    }

    printf("[INFO] cmd_del: Executing DEL for key: '%s'.\n", input->in.del_input.key);

    result.output.del_output.error = error;
    result.type = CMD_TYPE_DEL;
    return result;
}

static command_result_t cmd_exist(hashtable_t* context, command_data_t* input) {
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };

    if ((context == NULL) || (input == NULL) || (is_str_valid(input->in.exist_input.key))) {
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
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL) || (input->in.replace_input.key == NULL) || 
        (input->in.replace_input.new_value == NULL)){
        return result;
    }

    const unsigned char* key = input->in.replace_input.key;
    data_entry_t* new_value = (data_entry_t*)input->in.replace_input.new_value;

    fprintf(stderr, "[INFO] cmd_replace: Attempting to REPLACE value for key: '%s'.\n", key);

    int error_code = table_replace(context, key, new_value, destroy_data_entry);

    if (error_code != 0){
        fprintf(stderr, "[ERROR] cmd_replace: Failed to replace key '%s' (error code: %d). Key might not exist.\n", key, error_code);
        return result;
    }

    result.type = CMD_TYPE_REPLACE;
    result.output.replace_output.error = 0; 

    return result;
}

static command_result_t cmd_resize(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
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

static command_result_t cmd_clear(hashtable_t* context, command_data_t* call_id){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (call_id == NULL)){
        return result;
    }   

    fprintf(stderr, "[INFO] cmd_clear: Executing CLEAR for context: '%p'.\n", (void*)context);

    int error = table_clear(context, destroy_data_entry);
    if (error != 0){
        return result;
    }
    
    result.type = CMD_TYPE_CLEAR;
    result.output.clear_output.error = error;

    return result;
}

static command_result_t cmd_load_factor(hashtable_t* context, command_data_t* input){
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };

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
    command_result_t result = { .type = CMD_TYPE_ERROR, .output = {0} };
    if ((context == NULL) || (input == NULL)) {
        return result;
    }

    const cmd_count_t count_type = input->in.count_input.type;
    switch (count_type) {
        case CMD_COUNT_OCCUPIED_BUCKET: {
            double occupied = table_occupied_bucket_counter(context);
            result.output.count_output.type = CMD_COUNT_OCCUPIED_BUCKET;
            result.output.count_output.count_t.counter_d = occupied;
            break;
        }
        case CMD_COUNT_CAPACITY: {
            size_t capacity = table_capacity(context);
            result.output.count_output.type = CMD_COUNT_CAPACITY;
            result.output.count_output.count_t.counter_s = capacity;
            break;
        }
        case CMD_COUNT_MEMORY_USAGE: {
            size_t memory = table_memory_usage(context, std_value_sizer);
            result.output.count_output.type = CMD_COUNT_MEMORY_USAGE;
            result.output.count_output.count_t.counter_s = memory;
            break;
        }
        case CMD_COUNT_TOTAL_ELEM: {
            size_t total = table_total_elem(context);
            result.output.count_output.type = CMD_COUNT_TOTAL_ELEM;
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

static command command_table[] = { // Name needs to be in lexicographic order
    // name         tag                     proc               arity   flags
    //----------------------------------------------------------------------
    { "ADD",        CMD_TYPE_ADD,           cmd_add,           2,      "w" },
    { "CLEAR",      CMD_TYPE_CLEAR,         cmd_clear,         0,      "w" },
    { "COUNT",      CMD_TYPE_COUNT,         cmd_count,         1,      "r" },
    { "CREATE",     CMD_TYPE_CREATE,        cmd_create,        1,      "w" },
    { "DEL",        CMD_TYPE_DEL,           cmd_del,           1,      "w" },
    { "DESTROY",    CMD_TYPE_DESTROY,       cmd_destroy,       0,      "w" },
    { "EXIST",      CMD_TYPE_EXIST,         cmd_exist,         1,      "r" },
    { "GET",        CMD_TYPE_GET,           cmd_get,           1,      "r" },
    { "LOADFACTOR", CMD_TYPE_LOADFACTOR,    cmd_load_factor,   0,      "r" },
    { "REPLACE",    CMD_TYPE_REPLACE,       cmd_replace,       2,      "w" },
    { "RESIZE",     CMD_TYPE_RESIZE,        cmd_resize,        1,      "w" },
    { "SET",        CMD_TYPE_SET,           cmd_set,           2,      "w" },

    { NULL,         255,                    NULL,              0,      NULL}
};

// Command Table

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

data_type_t stotype(const char* type_str) {
    if ((type_str == NULL) || (type_str[0] == '\0')) {
        return DATA_TYPE_ERROR;
    }

    switch (toupper((unsigned char)type_str[0])) {
        case 'B':
            if (strcmp(type_str, "BOOL") == 0) return DATA_TYPE_BOOL;
            if (strcmp(type_str, "BIN") == 0) return DATA_TYPE_BIN;
            break; 

        case 'I':
            if (strcmp(type_str, "INT") == 0) return DATA_TYPE_INT;
            break;

        case 'S':
            if (strcmp(type_str, "STRING") == 0) return DATA_TYPE_STRING;
            if (strcmp(type_str, "SIZE") == 0) return DATA_TYPE_SIZE;
            break;

        case 'V':
            if (strcmp(type_str, "VOID_PTR") == 0) return DATA_TYPE_VOID_PTR;
            break;

        case 'D':
            if (strcmp(type_str, "DOUBLE") == 0) return DATA_TYPE_DOUBLE;
            if (strcmp(type_str, "DATA_ENTRY_PTR") == 0) return DATA_TYPE_DATA_ENTRY_PTR;
            break;

        case 'H':
            if (strcmp(type_str, "HASHTABLE_T_PTR") == 0) return DATA_TYPE_HASHTABLE_T_PTR;
            break;

        case 'E':
            if (strcmp(type_str, "EMPTY") == 0) return DATA_TYPE_EMPTY;
            if (strcmp(type_str, "ERROR") == 0) return DATA_TYPE_ERROR;
            break;

        default:
            break;
    }

    return DATA_TYPE_ERROR;
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

static int build_command_data(command_data_t* out_data, cmd_function_type tag, char* argv[]){
    out_data->tag = tag;

    switch (tag) {
        case CMD_TYPE_CREATE:{ 
            char* endptr;
            unsigned long value = strtoul(argv[0], &endptr, 10);
            if (endptr == argv[0] || *endptr != '\0'){
                fprintf(stderr, "[ERROR] build_command_data: Provided size for CREATE is not a valid number: '%s'.\n", argv[0]);
                return -1; 
            }
            out_data->in.create_input.db_size = (size_t)value;
            break;
        }
        
        case CMD_TYPE_SET:
        case CMD_TYPE_ADD:{
            const unsigned char* key = (const unsigned char*)argv[0];
            if (is_str_valid(key) == false){
                fprintf(stderr, "[ERROR] build_command_data: Provided key is not valid.\n");
                return -1;
            }
            char* endptr;
            unsigned long value_size = strtoul(argv[1], &endptr, 10);
            if (endptr == argv[1] || *endptr != '\0') {
                fprintf(stderr, "[ERROR] build_command_data: Provided size is not a valid number: '%s'.\n", argv[1]);
                return -1;
            }
            if (value_size > MAX_VALUE_SIZE){ 
                fprintf(stderr, "[ERROR] build_command_data: Provided value size exceeds the maximum allowed limit.\n");
                return -1;
            }
            data_type_t type = stotype(argv[2]);
            if (type == DATA_TYPE_ERROR){
                fprintf(stderr, "[ERROR] build_command_data: Provided data type is not valid or supported: '%s'.\n", argv[2]);
                return -1; 
            }
            data_entry_t* value_entry = malloc(sizeof(data_entry_t) + value_size + 1);
            if (value_entry == NULL){
                fprintf(stderr, "[ERROR] build_command_data: Memory allocation for value failed.\n");
                return -1;
            }

            value_entry->size = value_size;
            value_entry->type = type; 
            memcpy(value_entry->data, argv[3], value_size);
            value_entry->data[value_size] = '\0'; 
            out_data->in.set_input.key = key;
            out_data->in.set_input.value = value_entry;
            break;
        }

        case CMD_TYPE_GET:
        case CMD_TYPE_DEL:
        case CMD_TYPE_EXIST:{
            const unsigned char* key = (const unsigned char*)argv[0];
            if (is_str_valid(key) == false){
                fprintf(stderr, "[ERROR] build_command_data: Provided key is not valid.\n");
                return -1;
            }
            out_data->in.get_input.key = key;
            break;
        }

        case CMD_TYPE_REPLACE:{
            fprintf(stderr, "[ERROR] build_command_data: REPLACE command parsing not yet implemented.\n");
            return -1;
        }

        case CMD_TYPE_RESIZE:{
            char* endptr;
            unsigned long new_size = strtoul(argv[0], &endptr, 10);
            if (endptr == argv[0] || *endptr != '\0') {
                fprintf(stderr, "[ERROR] build_command_data: Provided size for RESIZE is not a valid number: '%s'.\n", argv[0]);
                return -1;
            }
            out_data->in.resize_input.new_size = (size_t)new_size;
            break;
        }

        case CMD_TYPE_DESTROY:
        case CMD_TYPE_CLEAR:
        case CMD_TYPE_LOADFACTOR: {
            out_data->in.destroy_input._dummy = 0;
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
        default:
            fprintf(stderr, "[ERROR] build_command_data: Received an unexpected or unsupported command tag: %d.\n", tag);
            return -1; 
    }

    return 0;
}

// PUBLIC API

execute_result_t execute_command(command_registry* reg, hashtable_t* context, 
                                 const char* command_name, int argc, char* argv[])
{
    execute_result_t result = { .content_type = DATA_TYPE_ERROR , .body_length = (size_t)0, .status_code = -1, .body = NULL };
    if ((context == NULL) || (reg == NULL) || (command_name == NULL) || 
        (argv != NULL)){
        return result;
    }
    
    for (int i = 0; i > argc; i++){
        if (argv[i] == NULL){
            return result;
        }
    }

    ssize_t command_index = find_command_index(reg, command_name);
    command command_struct = command_table[command_index];
    if (command_index == -1){
        return result;
    }

    if ((command_struct.arity != argc)){
        return result;
    }

}
