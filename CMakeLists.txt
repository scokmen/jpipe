cmake_minimum_required(VERSION 3.20)
project(jpipe VERSION 0.0.1 LANGUAGES C)

# Project Properties
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ENABLE_COVERAGE:
# Generates gcov instrumentation to track code execution path.
# Best used with Unit Tests to measure test quality.
option(ENABLE_COVERAGE "Enable Coverage" OFF)

# ENABLE_TSAN:
# Enables ThreadSanitizer to detect data races and deadlocks.
# Incompatible with ENABLE_ASAN. Heavily impacts performance; use for Stress Tests.
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

# ENABLE_ASAN:
# Enables AddressSanitizer to detect memory leaks, buffer overflows, 
# and use-after-free bugs. Incompatible with ENABLE_TSAN.
# Recommended for daily development and Unit Tests.
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if (ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASAN and TSAN cannot be enabled at the same time!")
endif ()

if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "Unsupported Compiler: (${CMAKE_C_COMPILER_ID})")
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
endif ()

find_package(Threads REQUIRED)

###############################################
# JPipe: Config Header
###############################################
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/gen/jp_config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/jp_config.h"
)

###############################################
# JPipe: Clang-Tidy
###############################################
find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
if (CLANG_TIDY_BINARY)
    message(STATUS "[clang-tidy]: ${CLANG_TIDY_BINARY}")
    set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY_BINARY} -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy)
endif ()

###############################################
# JPipe: Address Sanitizer
###############################################
if (ENABLE_ASAN)
    message(STATUS "[address-sanitizer]: enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_link_options(-fsanitize=address -static-libasan)
    else()
        add_link_options(-fsanitize=address)
    endif()
endif ()

###############################################
# JPipe: Thread Sanitizer
###############################################
if (ENABLE_TSAN)
    message(STATUS "[thread-sanitizer]: enabled")
    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif ()

###############################################
# JPipe: Coverage
###############################################
if (ENABLE_COVERAGE)
    message(STATUS "[coverage]: enabled")
    add_compile_options(--coverage)
    add_link_options(--coverage)
    find_program(LCOV_BINARY NAMES "lcov")
    find_program(GCOVR_BINARY NAMES "gcovr")
    if (LCOV_BINARY AND GCOVR_BINARY)
        add_custom_target(coverage
                COMMAND ${LCOV_BINARY} --capture --initial --directory . --output-file base.info
                COMMAND ${LCOV_BINARY} -capture --directory . --output-file test.info
                COMMAND ${LCOV_BINARY} --add-tracefile base.info --add-tracefile test.info --output-file total.info
                COMMAND ${LCOV_BINARY} --remove total.info '*/test/*' --output-file coverage_filtered.info
                COMMAND ${GCOVR_BINARY} -r ${CMAKE_SOURCE_DIR} . --exclude '.*/test/.*' --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif ()
endif ()

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
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
        PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/generated"
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
enable_testing()

add_library(jpipe_test_utils STATIC test/lib/jp_test.c)
target_link_libraries(jpipe_test_utils PUBLIC jpipe_core)
target_include_directories(jpipe_test_utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/test/lib")
target_compile_definitions(jpipe_test_utils PUBLIC JP_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/test/data")

file(GLOB TEST_FILES CONFIGURE_DEPENDS "test/unit/*.test.c")
foreach (test_source ${TEST_FILES})
    get_filename_component(file_name ${test_source} NAME_WE)
    set(test_name "unit_test_${file_name}")
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE jpipe_test_utils)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES LABELS "unit")
endforeach ()

file(GLOB TEST_FILES CONFIGURE_DEPENDS "test/stress/*.test.c")
foreach (test_source ${TEST_FILES})
    get_filename_component(file_name ${test_source} NAME_WE)
    set(test_name "stress_test_${file_name}")
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE jpipe_test_utils)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES LABELS "stress")
endforeach ()