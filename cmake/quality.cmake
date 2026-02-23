function(__discover_tests PATTERN TARGET LABEL)
    file(GLOB TEST_FILES CONFIGURE_DEPENDS "${PATTERN}")
    foreach (test_source ${TEST_FILES})
        get_filename_component(file_name ${test_source} NAME_WE)
        set(test_name "${LABEL}_test_${file_name}")
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} PRIVATE ${TARGET})
        add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
        message(STATUS "[tests]: included >> [type='${LABEL}']: ${test_name}")
        set_tests_properties(${test_name} PROPERTIES LABELS ${LABEL})
    endforeach ()
endfunction()

function(init_clang_tidy)
    find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
    if (CLANG_TIDY_BINARY)
        message(STATUS "[clang-tidy]: enabled  >> ${CLANG_TIDY_BINARY}")
        set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY_BINARY} -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy)
    else ()
        message(STATUS "[clang-tidy]: disabled >> [not found]")
    endif ()
endfunction()

function(init_coverage_flags TESTING COVERAGE)
    if (NOT TESTING)
        message(STATUS "[coverage]: disabled >> [test flag is off]")
        return()
    endif ()

    if (COVERAGE)
        add_link_options(--coverage)
        add_compile_options(--coverage)

        find_program(LCOV_BINARY NAMES "lcov")
        if (LCOV_BINARY)
            message(STATUS "[lcov]: enabled  >> ${LCOV_BINARY}")
        else ()
            message(STATUS "[lcov]: disabled >> [not found]")
        endif ()

        find_program(GCOVR_BINARY NAMES "gcovr")
        if (GCOVR_BINARY)
            message(STATUS "[gcovr]: enabled  >> ${GCOVR_BINARY}")
        else ()
            message(STATUS "[gcovr]: disabled >> [not found]")
        endif ()

        if (LCOV_BINARY AND GCOVR_BINARY)
            message(STATUS "[coverage]: enabled")
            add_custom_target(coverage
                    COMMAND ${LCOV_BINARY} --capture --initial --directory . --output-file base.info
                    COMMAND ${LCOV_BINARY} -capture --directory . --output-file test.info
                    COMMAND ${LCOV_BINARY} --add-tracefile base.info --add-tracefile test.info --output-file total.info
                    COMMAND ${LCOV_BINARY} --remove total.info '*/test/*' --output-file coverage_filtered.info
                    COMMAND ${GCOVR_BINARY} -r ${CMAKE_SOURCE_DIR} . --exclude '.*/test/.*' --print-summary
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
        else ()
            message(STATUS "[coverage]: disabled >> [lcov and gcovr must be available]")
        endif ()

    else ()
        message(STATUS "[coverage]: disabled")
    endif ()
endfunction()

function(init_tests TESTING LIBRARIES)
    if (TESTING)
        message(STATUS "[tests]: enabled")
        set(LIBRARY_NAME "jpipe_test_library")

        file(GLOB TEST_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/test/lib/*.c")
        add_library(${LIBRARY_NAME} STATIC ${TEST_FILES})

        target_link_libraries(${LIBRARY_NAME} PUBLIC ${LIBRARIES})
        target_include_directories(${LIBRARY_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/test/lib")
        target_compile_definitions(${LIBRARY_NAME} PUBLIC JP_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/test/data")

        __discover_tests("${CMAKE_SOURCE_DIR}/test/unit/*.test.c" ${LIBRARY_NAME} "unit")
        __discover_tests("${CMAKE_SOURCE_DIR}/test/stress/*.test.c" ${LIBRARY_NAME} "stress")
    else ()
        message(STATUS "[tests]: disabled")
    endif ()
endfunction()