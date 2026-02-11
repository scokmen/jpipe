cmake_minimum_required(VERSION 3.30)
project(jpipe VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

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
    target_link_libraries(${test_name} PRIVATE jpipe_test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()