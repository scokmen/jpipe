cmake_minimum_required(VERSION 3.20)
project(jpipe VERSION 0.0.1 LANGUAGES C)

# Project Properties.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print compiler information.
message(STATUS "[cc][compiler]: Compiler ID     : ${CMAKE_C_COMPILER_ID}")
message(STATUS "[cc][compiler]: Compiler Name   : ${CMAKE_C_COMPILER}")
message(STATUS "[cc][compiler]: Compiler Version: ${CMAKE_C_COMPILER_VERSION}")

# Extend the CMake module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include sub-modules.
include(CTest)
include(CheckIncludeFile)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(quality)
include(compilers)
include(sanitizers)
include(compatibility)

# Test compatibilities of;
# - Operating system.
# - Compiler attribute.
# - Standard library.
test_system_compatibility()

# Dependencies
find_package(Threads REQUIRED)

# ENABLE_TSAN:
# Enables 'Thread Sanitizer' to detect data races and deadlocks.
# Incompatible with ENABLE_ASAN. Heavily impacts performance; use for tests only.
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

# ENABLE_ASAN:
# Enables 'Address Sanitizer' to detect memory leaks, buffer overflows and use-after-free bugs. 
# Incompatible with ENABLE_TSAN. Heavily impacts performance; use for tests only.
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

###############################################
# JPipe: Code Generation
###############################################
configure_file(
        "${CMAKE_SOURCE_DIR}/include/jp_config.h.in"
        "${CMAKE_BINARY_DIR}/generated/jp_config.h"
        @ONLY
)

###############################################
# JPipe: Initializers
###############################################
add_library(jpipe_settings INTERFACE)
init_compiler_flags(jpipe_settings)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (ENABLE_ASAN AND ENABLE_TSAN)
        message(FATAL_ERROR "AddressSanitizer/ThreadSanitizer conflict!")
        return()
    endif ()

    if (ENABLE_ASAN)
        init_asan_flags(jpipe_settings)
    endif ()

    if (ENABLE_TSAN)
        init_tsan_flags(jpipe_settings)
    endif ()
endif ()

###############################################
# JPipe: Core
###############################################
add_subdirectory(src)

###############################################
# JPipe: Testing & Coverage
###############################################
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(test)
endif ()

###############################################
# JPipe: Micro Benchmarking
###############################################
if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_subdirectory(bench)
endif ()
