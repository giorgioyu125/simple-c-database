      
# Simple C In-Memory Key-Value Database

A lightweight, in-memory key-value database implemented in C. It utilizes a hash table with separate chaining for efficient data storage and retrieval. The database offers a straightforward command-line interface (CLI) for user interaction and supports optional data persistence to a file.

## Features

*   **In-Memory Storage**: Data is stored efficiently in memory using a hash table with separate chaining.
*   **Core Operations**: Supports fundamental CRUD operations via tcp request. (I suggest using netcat for testing)
*   **Command-Line Interface (CLI)**: Provides an interactive shell for easy database manipulation.
*   (TODO) **Data Persistence**: Offers `SAVE` and `LOAD` commands to persist data to and restore data from a file.
*   **C Implementation**: Written entirely in standard C (C23).
*   **CMake Build System**: Modern and flexible build process managed by CMake.

## Prerequisites

*   **CMake**: Version 3.10 or higher.
*   **C Compiler**: A C compiler supporting the C23 standard (e.g., GCC, Clang).

## Building the Project

This project uses CMake to manage the build process.

1.  Ensure you have the necessary tools for your Linux distribution to use CMake. It is also highly recommended to have a well-configured desktop and shell environment.
    ```
    sudo apt update
    sudo apt install build-essential cmake
    ```

2.  To ensure a clean build without obsolete files, it is good practice to remove the old build directory and clone the repository into an empty directory with `git clone <project_link>`. Alternatively, this command deletes the `build` folder and all its contents, preparing you for a reconfiguration from scratch.
    ```
    rm -rf build
    ```

3.  It is good practice to use a separate directory for compilation files, keeping the main project folder clean. This approach is known as an "out-of-source build," a practice I also follow in the project's development.
    ```
    mkdir build
    ```

4.  From the `/build` directory, run CMake. The `cmake ..` command tells CMake to look for the `CMakeLists.txt` file in the parent directory and to generate the necessary Makefiles for compilation inside the current `build` folder.
    ```
    cmake ..
    ```

5.  Now that the Makefiles are ready, run the `make` command. This will start the compilation process for all source files. Upon completion, you will find the final executable, named `simple_c_database`, directly in the `build` folder.
    Execute this command in the `build/` folder, which is located as a subdirectory of the cloned project repository:
    ```
    make
    ```

## Usage

You can simply execute and run the server with the command ./simple_c_database <BUCKET_NUMBER> in the build directory. The number of bucket is the number of high-speed unit preallocated in the database, they all can store a maximum of 8 values by default, but you can change this number in the MACRO section of the command.c in the part that says: #define BUCKET_CAPACITY 4. (Substitute 8 with the desidered number but 4 and 8 are the most reliable and efficent for simd optimization.)

---

## For Developers
      Hi dear Developer! If you are using clangd as LSP you'll obviusly need to create a symlink between the compile_commands.json in the build/ and the root dir. 
      There is no need to thank me!
### Contributing
Please contact me in private so we can discuss about your contribution. (Email: sabert148@gmail.com ,Discord: jonsnow0036)
    
