      
# Simple C In-Memory Key-Value Database

A lightweight, in-memory key-value database implemented in C. It utilizes a hash table with separate chaining for efficient data storage and retrieval. The database offers a straightforward command-line interface (CLI) for user interaction and supports optional data persistence to a file.

## Features

*   **In-Memory Storage**: Data is stored efficiently in memory using a hash table with separate chaining.
*   **Core Operations**: Supports fundamental CRUD operations via http request:
    *   `SET <Key> <Datatype:Value>` (Type `help datatypes` for a list of supported data types)
    *   `GET <Key>`
    *   `DEL <Key>`
    *   `EXISTS <Key>`
*   **Command-Line Interface (CLI)**: Provides an interactive shell for easy database manipulation.
*   **Data Persistence**: Offers `SAVE` and `LOAD` commands to persist data to and restore data from a file.
*   **C Implementation**: Written entirely in standard C (C23).
*   **CMake Build System**: Modern and flexible build process managed by CMake.

## Prerequisites

*   **CMake**: Version 3.10 or higher.
*   **C Compiler**: A C compiler supporting the C23 standard (e.g., GCC, Clang).

## Building the Project

This project uses CMake to manage the build process.

1.  **Clone the repository (if you haven't already):**
    ```bash
    git clone <your-repository-url>
    cd <repository-directory>
    ```

2.  **Create a build directory and navigate into it:**
    It's good practice to build outside the source directory.
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure the project with CMake:**
    This step generates the build files (e.g., Makefiles).
    ```bash
    cmake ..
    ```

4.  **Compile the project:**
    ```bash
    cmake --build .
    # Or if you have already generated makefiles:
    # make
    ```

5.  **Run the executable:**
    ```bash
    ./your_executable_name
    # Example: ./key_value_db
    ```

## Usage

Once built, run the executable to start the CLI. You can then use the commands listed in the "Features" section. Type `HELP` within the CLI for a list of available commands and `HELP DATATYPES` for information on supported data types.

---

## For Developers (Optional Sections you might want to add later)

### Contributing
Please contact me in private so we can discuss about your contribution. (Email: sabert148@gmail.com ,Discord: jonsnow0036)
    
