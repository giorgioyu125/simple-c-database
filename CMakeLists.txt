cmake_minimum_required(VERSION 3.14) 

project(
    simple_c_database
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_NAME simple_c_database)

add_executable(${EXECUTABLE_NAME}
    src/server.c
    src/command.c
    src/hashing_functionality.c
    src/hashtable.c
    src/string_functionality.c
    src/bitwise_functionality.c
)

target_compile_definitions(${EXECUTABLE_NAME}
    PRIVATE _POSIX_C_SOURCE=200809L
)

target_compile_options(${EXECUTABLE_NAME} PRIVATE
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
    -luv
)

option(ENABLE_WERROR "Treat warnings as errors" OFF)
if(ENABLE_WERROR)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Werror)
    message(STATUS "Treating warnings as errors (-Werror) is ENABLED")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Enabling Address and Undefined Behavior sanitizers")
    target_compile_options(${EXECUTABLE_NAME} PRIVATE
        -O0 
        -fsanitize=address
        -fsanitize=undefined
    )
    target_link_options(${EXECUTABLE_NAME} PRIVATE
        -fsanitize=address
        -fsanitize=undefined
    )
endif()


find_package(PkgConfig REQUIRED)
pkg_check_modules(UV REQUIRED IMPORTED_TARGET libuv)

find_package(Threads REQUIRED)


target_include_directories(${EXECUTABLE_NAME}
    PRIVATE 
        src
)

# Collega le librerie necessarie
target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        Threads::Threads
        PkgConfig::UV     
)

message(STATUS "Configuration complete. Use 'cmake --build .' to build.")
