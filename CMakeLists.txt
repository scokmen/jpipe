cmake_minimum_required(VERSION 3.20)
project(jpipe VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################################
# JPipe: Options
###############################################
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_COVERAGE "Enable Coverage" OFF)

###############################################
# JPipe: Sanitizer / Coverage Flags
###############################################
if(ENABLE_ASAN)
    message(STATUS "[address-sanitizer]: enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_COVERAGE)
    message(STATUS "[coverage]: enabled")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

###############################################
# JPipe: Executable
###############################################
add_executable(jpipe
        src/main.c
        src/errno.c
        src/options.c
)

target_include_directories(jpipe PRIVATE include)

###############################################
# JPipe: Tests
###############################################
enable_testing()

add_library(jpipe_test STATIC
        src/options.c
        src/errno.c
)

target_include_directories(jpipe_test
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/test/lib
)

file(GLOB TEST_FILES CONFIGURE_DEPENDS "test/unit/*_test.c")

foreach(test_source ${TEST_FILES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE jpipe_test)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

###############################################
# JPipe: Coverage
###############################################
if(ENABLE_COVERAGE)
    find_program(LCOV_BINARY NAMES "lcov")
    find_program(GCOVR_BINARY NAMES "gcovr")
    
    if(LCOV_BINARY AND GCOVR_BINARY)
        add_custom_target(coverage
                COMMAND ${LCOV_BINARY} --capture --initial --directory . --output-file base.info
                COMMAND ${LCOV_BINARY} -capture --directory . --output-file test.info
                COMMAND ${LCOV_BINARY} --add-tracefile base.info --add-tracefile test.info --output-file total.info
                COMMAND ${LCOV_BINARY} --remove total.info '*/test/*' --output-file coverage_filtered.info
                COMMAND ${GCOVR_BINARY} -r ${CMAKE_SOURCE_DIR} . --exclude '.*/test/.*' --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

###############################################
# JPipe: Clang-Tidy
###############################################
find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
if(CLANG_TIDY_BINARY)
    message(STATUS "[clang-tidy]: ${CLANG_TIDY_BINARY}")
    set_target_properties(jpipe PROPERTIES C_CLANG_TIDY "${CLANG_TIDY_BINARY}")
endif()
