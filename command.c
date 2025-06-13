// HEADER

#include "command.h"
#include "string_functionality.h"
#include <stdio.h>

// PRIVATE API
            
:%s/first_argument\[0\]/argv[1]/gc

    
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

static int cmd_init(hashtable_t** context, int argc, char** argv) {
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
    {   "REPLACE",      cmd_replace,            4,      "rw"    },
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
    if (!reg) {
        fprintf(stderr, "[ERROR] registry_create: Failed to allocate memory for command registry");
        return NULL;
    }

    size_t command_count = (sizeof(command_table) / sizeof(command)) - 1;

    reg->commands = command_table;
    reg->count = command_count;

    printf("[INFO] registry_create: Command registry created with %zu commands.\n", reg->count);

    return reg;
}

int registry_destroy(command_registry** reg){
    if (reg == NULL){
        return -1;
    }
    
    (*reg)->count = 0;
    (*reg)->commands = NULL;

    free(*reg);

    return 0;
}


ssize_t find_command_index_from_line(command_registry* reg, const char* line){
    if (!reg || !line) {
        return -1; 
    }

    char command_token[KEY_MAX_LEN]; 
    int i = 0;

    while (isspace((unsigned char)*line)) {
        line++;
    }

    while (*line && !isspace((unsigned char)*line) && i < sizeof(command_token) - 1) {
        command_token[i++] = *line++;
    }
    command_token[i] = '\0'; 

    if (i == 0) {
        return -1; 
    }

    ssize_t found_index = -1;

    for (size_t j = 0; j < reg->count; ++j) {
        if (if (strcasecmp(command_token, reg->commands[j].name) == 0)) {
            found_index = j; 
            break;
        }
    }

    return found_index;
}


// PUBLIC API

int execute_cmd(command_registry* reg, hashtable_t* db, char* line) {
    if (!reg || !db || !line) {
        return -1;
    }

    ssize_t command_index = find_command_index_from_line(reg, line);

    if (command_index == -1) {
        fprintf(stderr, "[ERRORE] Comando non riconosciuto.\n");
        return -1;
    }

    const command* cmd = registry_get_command_by_index(reg, command_index);
    if (cmd == NULL) {
        fprintf(stderr, "[ERRORE] Errore interno: indice di comando non valido.\n");
        return -1;
    }

    size_t argc = 0;
    char** argv = split_string_by_space(line, &argc);
    if (argv == NULL) {
        fprintf(stderr, "[ERRORE] Fallimento durante il parsing degli argomenti.\n");
        return -1;
    }

    if (cmd->arity != argc) {
        fprintf(stderr, "[ERRORE] Numero di argomenti errato per '%s'. Attesi %d, ricevuti %zu.\n",
                cmd->name, cmd->arity, argc);
        free(argv);
        return -1;
    }

    if (cmd->proc == NULL) {
        fprintf(stderr, "[ERRORE] Comando '%s' non implementato.\n", cmd->name);
        free(argv);
        return -1;
    }

    printf("[INFO] Esecuzione di '%s'...\n", cmd->name);
    int result = cmd->proc(db, argc, argv);

    free(argv);

    return result;
}
