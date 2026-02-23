cmake_minimum_required(VERSION 3.20)
project(jpipe VERSION 0.0.1 LANGUAGES C)

# Project Properties.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# This project supports only GNU GCC and Clang compilers.
if (NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "Unsupported Compiler: (${CMAKE_C_COMPILER_ID})")
endif ()

# Extend the CMake module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include sub modules.
include(quality)
include(sanitizers)

# ENABLE_TEST:
# Discovers all unit and stress tests.
option(ENABLE_TEST "Enable Test" OFF)

# ENABLE_COVERAGE:
# Generates gcov instrumentation to track code execution path.
option(ENABLE_COVERAGE "Enable Coverage" OFF)

# ENABLE_TSAN:
# Enables ThreadSanitizer to detect data races and deadlocks.
# Incompatible with ENABLE_ASAN. Heavily impacts performance; use for tests only.
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

# ENABLE_ASAN:
# Enables AddressSanitizer to detect memory leaks, buffer overflows and use-after-free bugs. 
# Incompatible with ENABLE_TSAN. Heavily impacts performance; use for tests only.
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

###############################################
# JPipe: Initializers
###############################################
init_clang_tidy()
init_coverage_flags(${ENABLE_TEST} ${ENABLE_COVERAGE})
init_sanitizer_flags(${ENABLE_ASAN} ${ENABLE_TSAN})

###############################################
# JPipe: Compile Options & Packages
###############################################
if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
endif ()

find_package(Threads REQUIRED)

###############################################
# JPipe: Generated Headers
###############################################
configure_file(
        "${CMAKE_SOURCE_DIR}/gen/jp_config.h.in"
        "${CMAKE_BINARY_DIR}/generated/jp_config.h"
)

###############################################
# JPipe: Core Library
###############################################
add_library(jpipe_core STATIC
        src/errno.c
        src/command.c
        src/worker.c
        src/field.c
        src/queue.c
)

target_include_directories(jpipe_core
        PUBLIC "${CMAKE_SOURCE_DIR}/include"
        PUBLIC "${CMAKE_BINARY_DIR}/generated"
)

target_link_libraries(jpipe_core PUBLIC Threads::Threads)

###############################################
# JPipe: Executable
###############################################
add_executable(jpipe src/main.c)

target_link_libraries(jpipe PRIVATE jpipe_core)

###############################################
# JPipe: Unit Tests
###############################################
if (ENABLE_TEST)
    enable_testing()
    init_tests(${ENABLE_TEST} jpipe_core)
endif ()

