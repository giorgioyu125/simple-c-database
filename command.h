// command.h

#ifndef COMMAND_H
#define COMMAND_H

// INCLUDES

#include <stdio.h>
#include <strings.h>
#include "hashtable.h"
#include "string_functionality.h"


// MACRO

typedef int (*command_proc)(hashtable_t* context, int argc, char **argv);

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
command* find_command(const command_registry* reg, const char* name);


#endif
