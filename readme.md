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

1. Requires a C compiler (like GCC). Compile all `.c` files together.
2. Make the compiled file an executable (ex: chmod +x <filename>).
3. Run the executable!
