function(init_clang_tidy TARGET)
    if (NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(WARNING "[clang-tidy]: disabled >> [cc=${CMAKE_C_COMPILER_ID}]")
        return()
    endif ()
    find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
    if (CLANG_TIDY_BINARY)
        execute_process(
                COMMAND ${CLANG_TIDY_BINARY} --version
                OUTPUT_VARIABLE CLANG_TIDY_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(CLANG_TIDY_COMMAND
                "${CLANG_TIDY_BINARY}"
                "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
                "--extra-arg=-Wno-unknown-warning-option"
                "--extra-arg=-Wno-ignored-optimization-argument"
        )
        set_target_properties(${TARGET} PROPERTIES
                C_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )
        message(STATUS "[clang-tidy]: enabled
            version=${CLANG_TIDY_VERSION}
            path=${CLANG_TIDY_BINARY}"
        )
    else ()
        message(WARNING "[clang-tidy]: disabled >> [not found]")
    endif ()
endfunction()

function(init_clang_format)
    find_program(CLANG_FORMAT_BINARY clang-format)
    if (CLANG_FORMAT_BINARY)
        execute_process(
                COMMAND ${CLANG_FORMAT_BINARY} --version
                OUTPUT_VARIABLE CLANG_FORMAT_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        file(GLOB_RECURSE ALL_SOURCE_FILES
                "${CMAKE_SOURCE_DIR}/src/*.c"
                "${CMAKE_SOURCE_DIR}/include/*.h")
        add_custom_target(format
                COMMAND ${CLANG_FORMAT_BINARY}
                --style=file
                --dry-run
                --Werror
                -n
                ${ALL_SOURCE_FILES}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "[clang-format]: analysing..."
        )
        message(STATUS "[clang-format]: enabled
            version=${CLANG_FORMAT_VERSION}
            path=${CLANG_FORMAT_BINARY}"
        )
    else ()
        message(WARNING "[clang-format]: disabled >> [not found]")
    endif ()
endfunction()

function(init_coverage_flags TARGET)
    find_program(LCOV_BINARY NAMES "lcov")
    if (LCOV_BINARY)
        execute_process(
                COMMAND ${LCOV_BINARY} --version
                OUTPUT_VARIABLE LCOV_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        message(STATUS "[lcov]: enabled
            version=${LCOV_VERSION}
            path=${LCOV_BINARY}"
        )
    else ()
        message(WARNING "[lcov]: disabled >> [not found]")
    endif ()

    find_program(GCOVR_BINARY NAMES "gcovr")
    if (GCOVR_BINARY)
        execute_process(
                COMMAND ${GCOVR_BINARY} --version
                OUTPUT_VARIABLE GCOVR_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        message(STATUS "[gcovr]: enabled
            version=${GCOVR_VERSION}
            path=${GCOVR_BINARY}"
        )
    else ()
        message(WARNING "[gcovr]: disabled >> [not found]")
    endif ()

    if (NOT (LCOV_BINARY AND GCOVR_BINARY))
        message(WARNING "[coverage]: disabled >> [lcov and gcovr must be available]")
        return()
    endif ()

    target_compile_options(${TARGET} INTERFACE
            --coverage
    )
    target_link_options(${TARGET} INTERFACE
            --coverage
    )

    if (NOT TARGET coverage)
        add_custom_target(coverage
                COMMAND ${LCOV_BINARY} --capture --initial --directory . --output-file base.info
                COMMAND ${LCOV_BINARY} -capture --directory . --output-file test.info
                COMMAND ${LCOV_BINARY} --add-tracefile base.info --add-tracefile test.info --output-file total.info
                COMMAND ${LCOV_BINARY} --remove total.info '*/test/*' --output-file coverage_filtered.info
                COMMAND ${GCOVR_BINARY} -r ${CMAKE_SOURCE_DIR} . --exclude '.*/test/.*' --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "[coverage]: analysing..."
        )
    endif ()
    message(STATUS "[coverage]: enabled")
endfunction()