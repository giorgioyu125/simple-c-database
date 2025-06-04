cmake_minimum_required(VERSION 3.10)

project(simple-c-database C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

set(LIBUV_FOUND_VIA_CMAKE_CONFIG FALSE)
set(LIBUV_FOUND_VIA_PKGCONFIG FALSE)

find_package(libuv QUIET)

if(TARGET libuv::libuv)
    message(STATUS "Found libuv via CMake config (libuv::libuv target).")
    set(LIBUV_FOUND_VIA_CMAKE_CONFIG TRUE)
else()
    message(STATUS "libuv::libuv target not found. Attempting to find libuv using pkg-config.")

    find_package(PkgConfig QUIET) 
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_LIBUV QUIET libuv) 
        if(PC_LIBUV_FOUND)
            message(STATUS "Found libuv via pkg-config.")
            set(LIBUV_FOUND_VIA_PKGCONFIG TRUE)
        else()
            message(STATUS "libuv not found via pkg-config.")
        endif()
    else()
        message(STATUS "pkg-config tool not found. Cannot use it to find libuv as a fallback.")
    endif()
endif()

if(NOT LIBUV_FOUND_VIA_CMAKE_CONFIG AND NOT LIBUV_FOUND_VIA_PKGCONFIG)
    message(FATAL_ERROR "REQUIRED package libuv could not be found by CMake config (libuv::libuv) or pkg-config.")
endif()

add_executable(simple-c-database
    main.c
    cli.c
    hashing_functionality.c
    hashset.c
    hashtable.c
    string_functionality.c
)

target_include_directories(simple-c-database PRIVATE
    ${PROJECT_SOURCE_DIR}
)

target_compile_options(simple-c-database PRIVATE
    -O0                
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
    -Werror           
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simple-c-database PRIVATE
        -fsanitize=address
        -fsanitize=undefined
    )
    target_link_options(simple-c-database PRIVATE
        -fsanitize=address
        -fsanitize=undefined
    )
endif()

if(LIBUV_FOUND_VIA_CMAKE_CONFIG)
    message(STATUS "Linking with libuv::libuv.")
    target_link_libraries(simple-c-database PRIVATE libuv::libuv)
elseif(LIBUV_FOUND_VIA_PKGCONFIG)
    message(STATUS "Linking with libuv found via pkg-config.")

    if(PC_LIBUV_INCLUDE_DIRS)
        target_include_directories(simple-c-database PRIVATE ${PC_LIBUV_INCLUDE_DIRS})
    endif()

    if(PC_LIBUV_CFLAGS_OTHER)
        target_compile_options(simple-c-database PRIVATE ${PC_LIBUV_CFLAGS_OTHER})
    endif()

    if(PC_LIBUV_LIBRARY_DIRS)
        link_directories(${PC_LIBUV_LIBRARY_DIRS})
    endif()

    if(PC_LIBUV_LDFLAGS_OTHER)
        target_link_options(simple-c-database PRIVATE ${PC_LIBUV_LDFLAGS_OTHER})
    endif()

    if(PC_LIBUV_LIBRARIES)
        target_link_libraries(simple-c-database PRIVATE ${PC_LIBUV_LIBRARIES})
    endif()
else()

    message(FATAL_ERROR "libuv was marked as found, but no linkage method is available. This indicates a bug in CMakeLists.txt logic.")
endif()

add_compile_definitions(_POSIX_C_SOURCE=200809L)
