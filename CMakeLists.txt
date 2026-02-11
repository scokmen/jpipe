cmake_minimum_required(VERSION 3.30)
project(jpipe VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################################
# JPipe: Code Quality
###############################################
find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
if(CLANG_TIDY_BINARY)
    message(STATUS "[clang-tidy]: ${CLANG_TIDY_BINARY}")
    set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY_BINARY})
else()
    message(WARNING "[clang-tidy]: cannot be found")
endif()

###############################################
# JPipe: Executable
###############################################
include_directories(include)

set(SOURCES 
        src/main.c
        src/errno.c
        src/options.c
)

add_executable(jpipe ${SOURCES})

###############################################
# JPipe: Tests
###############################################
enable_testing()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(--coverage -fsanitize=address -fno-omit-frame-pointer)
    add_link_options(--coverage -fsanitize=address)
endif()

set(TEST_SOURCES
        src/options.c
        src/errno.c
)

add_library(jpipe_test STATIC ${TEST_SOURCES})
target_include_directories(jpipe_test PRIVATE include test/lib)
file(GLOB TEST_FILES "test/unit/*_test.c")

foreach(test_source ${TEST_FILES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    set_target_properties(${test_name} PROPERTIES C_CLANG_TIDY "")
    target_link_libraries(${test_name} PRIVATE jpipe_test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

find_program(LCOV_BINARY NAMES "lcov")
find_program(GCOVR_BINARY NAMES "gcovr")
if(GCOVR_BINARY)
    add_custom_target(coverage
            COMMAND ${LCOV_BINARY} 
                --capture --directory . 
                --output-file coverage.info 
                --ignore-errors gcov,gcov
            COMMAND ${LCOV_BINARY} 
                --remove coverage.info '*/test/*'
                --output-file coverage_filtered.info 
                --ignore-errors gcov,gcov
            COMMAND ${GCOVR_BINARY} -r ${CMAKE_SOURCE_DIR} . 
                --object-directory .
                --exclude '.*/test/.*'
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()