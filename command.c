// HEADER

#include "command.h"
#include "string_functionality.h"
#include <stdio.h>

// PRIVATE API
    
static int cmd_set(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_set: Executing SET for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_set(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_set: Executing SET for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_get(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_get: Executing GET for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_add(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_add: Executing ADD for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_init(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_init: Executing INIT for size: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_destroy(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_destroy: Executing DESTROY.\n");
    return 0;
}

static int cmd_del(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_del: Executing DEL for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_exist(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_exist: Executing EXIST for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_replace(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_repl: Executing REPLACE for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_resize(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_resize: Executing RESIZE for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_clear(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_clear: Executing CLEAR for key: '%s'.\n", argv[1]);
    return 0;
}

static int cmd_load_factor(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_load_factor: Executing LOAD_FACTOR for key: '%s'\n", argv[1]);
    return 0;
}

static int cmd_count(hashtable_t* context, int argc, char** argv) {
    printf("[INFO] cmd_count: Executing COUNT of type '%s'.\n", argv[1]);
    return 0;
}


static struct command command_table[] = {
    //  Name            Function Pointer        Arity   R/W
    {   "GET",          cmd_get,                2,      "r"     },
    {   "SET",          cmd_set,                3,      "w"     },
    {   "ADD",          cmd_add,                3,      "rw"    },
    {   "INIT",         cmd_init,               1,      "w"     },
    {   "DESTROY",      cmd_destroy,            1,      "rw"    },
    {   "DEL",          cmd_del,                3,      "rw"    },
    {   "EXIST",        cmd_exist,              2,      "r"     },
    {   "REPLACE",      cmd_repl,               4,      "rw"    },
    {   "RESIZE",       cmd_resize,             2,      "rw"    },
    {   "CLEAR",        cmd_clear,              2,      "w"     },
    {   "LOAD_FACTOR",  cmd_load_factor,        1,      "r"     },
    {   "COUNT",        cmd_count,              1,      "r"     },

    {   NULL,           NULL,                   0,      NULL    }
};

struct command_registry {
    struct command* commands;
    size_t count;
};


// CMD Helper Functions

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


ssize_t find_command_index(command_registry* reg, const char* command){
    if ((reg == NULL) || (line == NULL) {
        return -1; 
    }
    
    for (size_t i = 0; i < reg->count; i++){
         if (ustrcmp(reg->commands[i].name, command_name) == 0) {
            return i;
        }       
    }

    return -1;
}


// PUBLIC API

int execute_cmd(const command_registry* reg, hashtable_t* db, char* line) {
    if (reg == NULL || db == NULL || line == NULL) {
        return -1;
    }

    char* tokens[MAX_TOKENS];
    int num_tokens = tokenize_string(" ", line, MAX_TOKENS, tokens);

    if (num_tokens <= 0) {
        return -1; 
    }

    const char* command_name = tokens[0];
    ssize_t command_index = find_command_index(reg, command_name);
    if (command_index == -1) {
        return -1; 
    }

    const struct command* cmd = reg->commands[command_index];

    if (cmd->arity != num_tokens) {
        return -1; 
    }

    return cmd->proc(db, num_tokens, tokens);
}
