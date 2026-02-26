cmake_minimum_required(VERSION 3.20)
project(jpipe VERSION 0.0.1 LANGUAGES C)

# Project Properties.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Extend the CMake module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include sub-modules.
include(CTest)
include(quality)
include(compilers)
include(sanitizers)

# Dependencies
find_package(Threads REQUIRED)

# ENABLE_TSAN:
# Enables ThreadSanitizer to detect data races and deadlocks.
# Incompatible with ENABLE_ASAN. Heavily impacts performance; use for tests only.
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

# ENABLE_ASAN:
# Enables AddressSanitizer to detect memory leaks, buffer overflows and use-after-free bugs. 
# Incompatible with ENABLE_TSAN. Heavily impacts performance; use for tests only.
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

###############################################
# JPipe: Generated Headers
###############################################
configure_file(
        "${CMAKE_SOURCE_DIR}/gen/jp_config.h.in"
        "${CMAKE_BINARY_DIR}/generated/jp_config.h"
)

###############################################
# JPipe: Initializers
###############################################
add_library(jpipe_settings INTERFACE)

init_compiler_flags(jpipe_settings)

message(STATUS "[build]: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    init_debug_flags(jpipe_settings)
    
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

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_TESTING OFF CACHE BOOL "[testing]: disabled" FORCE)
    init_release_flags(jpipe_settings)
endif ()


###############################################
# JPipe: Packages
###############################################
add_subdirectory(src)

if (BUILD_TESTING)
    init_coverage_flags(jpipe_settings)
    add_subdirectory(test)
endif ()
