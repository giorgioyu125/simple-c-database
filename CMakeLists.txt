cmake_minimum_required(VERSION 3.10)

project(simple-c-database C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(simple-c-database
    main.c
    cli.c
    hashing_functionality.c
    hashset.c
    hashtable.c
    string_functionality.c
)

target_include_directories(simple-c-database PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(simple-c-database PRIVATE
    -g3
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wformat=2
    -Wformat-security
    -Werror                     #Comment this section for non-development purpouse
)

target_link_libraries(simple-c-database PRIVATE
    -fsanitize=address
    -fsanitize=undefined
)

    
