# Simple C In-Memory Key-Value Database

A basic key-value database implemented in C. It stores data in memory using a hash table with separate chaining and provides a simple command-line interface (CLI) for interaction. Includes optional file persistence.

## Features

*   In-memory storage using a hash table (separate chaining).
*   Basic CRUD operations: `SET <Key> <Datatype:Value>` (you can get a list of all Datatype by typing help datatypes), `GET <Key>`, `DEL <Key>`, `EXISTS <Key>`.
*   Command-Line Interface (CLI) for interaction.
*   Optional persistence: `SAVE` data to a file, `LOAD` data from a file.
*   Written purely in C.
*   Debug mode via compile-time flag (`DEBUG_MODE`).

## Building

When we talk about building this project we have mainly two ways of doing that, the first is for tester/developer only, the second for regular users.


### First Method:
    1. Requires a C compiler that supports c23 (like GCC). *RUN the build.sh* script.
    2. Make the compiled file an executable (ex: chmod +x <filename>).
    3. Run the executable or put it in a debugger like gdb.


### Second Method:
    1. Requires a C compiler that supports c23 (like GCC). Compile all `.c` files together.
    2. Make the compiled file an executable (ex: chmod +x <filename>).
    3. Run the executable!
