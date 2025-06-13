// command.h

#ifndef COMMAND_H
#define COMMAND_H

// INCLUDES

#include <strings.h>
#include "hashtable.h"


// MACRO

typedef int (*command_proc)(hashtable_t* context, int argc, char* command_argument);

// DATA

typedef struct command{
    const char* name;
    command_proc proc;
    int arity;
    const char* flags;
} command;

typedef struct command_registry command_registry;

// API

command_registry* registry_create();
void registry_destroy(command_registry* reg);
ssize_t find_command_index_from_line(command_registry* reg, const char* line);

#endif
