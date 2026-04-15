function(__get_binary_version BINARY_PATH FLAG VERSION)
    execute_process(
            COMMAND ${BINARY_PATH} ${FLAG}
            OUTPUT_VARIABLE COMMAND_OUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${VERSION} "${COMMAND_OUT}" PARENT_SCOPE)
endfunction()

function(init_clang_tidy TARGET)
    if (NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(WARNING "[clang-tidy]: skipped >> [cc=${CMAKE_C_COMPILER_ID}]")
        return()
    endif ()

    find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
    if (CLANG_TIDY_BINARY)
        __get_binary_version(
                ${CLANG_TIDY_BINARY}
                "--version"
                CLANG_TIDY_VERSION
        )
        message(STATUS "[clang-tidy]: found >> version=v${CLANG_TIDY_VERSION}, path='${CLANG_TIDY_BINARY}'")

        set(CLANG_TIDY_COMMAND
                "${CLANG_TIDY_BINARY}"
                "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
                "--extra-arg=-Wno-unknown-warning-option"
                "--extra-arg=-Wno-ignored-optimization-argument"
        )
        set_target_properties(${TARGET} PROPERTIES
                C_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )
        message(STATUS "[clang-tidy]: activated")
    else ()
        message(WARNING "[clang-tidy]: skipped >> [not found]")
    endif ()
endfunction()

function(init_clang_format)
    find_program(CLANG_FORMAT_BINARY clang-format)
    if (CLANG_FORMAT_BINARY)
        __get_binary_version(
                ${CLANG_FORMAT_BINARY}
                "--version"
                CLANG_FORMAT_VERSION
        )
        message(STATUS "[clang-format]: found >> version=v${CLANG_FORMAT_VERSION}, path='${CLANG_FORMAT_BINARY}'")

        if (NOT TARGET format)
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
            message(STATUS "[clang-format]: targeted >> [target=format]")
        endif ()
    else ()
        if (NOT TARGET format)
            add_custom_target(format
                    COMMAND ${CMAKE_COMMAND} -E echo "[clang-format]: skipped >> [not found]"
                    COMMENT "[clang-format]: skipping..."
            )
        endif ()
        message(WARNING "[clang-format]: skipped >> [not found]")
    endif ()
endfunction()

function(init_cppcheck)
    find_program(CPPCHECK_BINARY cppcheck)
    if (CPPCHECK_BINARY)
        __get_binary_version(
                ${CPPCHECK_BINARY}
                "--version"
                CPPCHECK_VERSION
        )
        message(STATUS "[cppcheck]: found >> version=v${CPPCHECK_VERSION}, path='${CPPCHECK_BINARY}'")

        set(CPPCHECK_BASE_FLAGS
                --project=${CMAKE_BINARY_DIR}/compile_commands.json
                --inline-suppr
                --library=std
                --library=posix
                --library=${CMAKE_SOURCE_DIR}/cppcheck/cppcheck.cfg.xml
                --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck/.cppcheck_suppressions
                --check-level=exhaustive
                --force
                --quiet
        )

        if (NOT TARGET check-and-inform)
            add_custom_target(check-and-inform
                    COMMAND ${CPPCHECK_BINARY}
                    ${CPPCHECK_BASE_FLAGS}
                    --enable=style,performance,portability,warning,information
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "[cppcheck/inform]: checking..."
            )
            message(STATUS "[cppcheck/inform]: targeted >> [target=check-and-inform]")
        endif ()
        if (NOT TARGET check-and-enforce)
            add_custom_target(check-and-enforce
                    COMMAND ${CPPCHECK_BINARY}
                    ${CPPCHECK_BASE_FLAGS}
                    --enable=warning,performance,portability
                    --error-exitcode=1
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "[cppcheck/enforce]: checking..."
            )
            message(STATUS "[cppcheck/enforce]: targeted >> [target=check-and-enforce]")
        endif ()
    else ()
        if (NOT TARGET check-and-inform)
            add_custom_target(check-and-inform
                    COMMAND ${CMAKE_COMMAND} -E echo "[cppcheck/inform]: skipped >> [not found]"
                    COMMENT "[cppcheck/inform]: skipping..."
            )
        endif ()
        if (NOT TARGET check-and-enforce)
            add_custom_target(check-and-enforce
                    COMMAND ${CMAKE_COMMAND} -E echo "[cppcheck/enforce]: skipped >> [not found]"
                    COMMENT "[cppcheck/enforce]: skipping..."
            )
        endif ()
        message(WARNING "[cppcheck]: skipped >> [not found]")
    endif ()
endfunction()

function(init_test LABEL SOURCE LIBRARIES)
    get_filename_component(raw_name ${SOURCE} NAME_WE)
    set(test_target "${LABEL}_test_${raw_name}")
    add_executable(${test_target} ${SOURCE})
    target_link_libraries(${test_target} PRIVATE ${LIBRARIES})
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES LABELS ${LABEL})
    message(STATUS "[test]: ${test_target}")
endfunction()

function(init_coverage_flags TARGET)
    find_program(LCOV_BINARY NAMES "lcov")
    if (LCOV_BINARY)
        __get_binary_version(
                ${LCOV_BINARY}
                "--version"
                LCOV_VERSION
        )
        message(STATUS "[lcov]: found >> version=v${LCOV_VERSION}, path='${LCOV_BINARY}'")
    else ()
        message(WARNING "[lcov]: skipped >> [not found]")
    endif ()

    find_program(GCOVR_BINARY NAMES "gcovr")
    if (GCOVR_BINARY)
        __get_binary_version(
                ${GCOVR_BINARY}
                "--version"
                GCOVR_VERSION
        )
        message(STATUS "[gcovr]: found >> version=v${GCOVR_VERSION}, path='${GCOVR_BINARY}'")
    else ()
        message(WARNING "[gcovr]: skipped >> [not found]")
    endif ()

    if (NOT (LCOV_BINARY AND GCOVR_BINARY))
        if (NOT TARGET coverage)
            add_custom_target(coverage
                    COMMAND ${CMAKE_COMMAND} -E echo "[coverage]: skipped >> [lcov || gcovr missing]"
                    COMMENT "[coverage]: skipping..."
            )
        endif ()
        message(WARNING "[coverage]: skipped >> [lcov || gcovr missing]")
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
                COMMAND ${LCOV_BINARY} --remove total.info '/usr/*' '*/test/*' --output-file coverage_filtered.info
                COMMAND ${GCOVR_BINARY} -r ${CMAKE_SOURCE_DIR} . --exclude '.*/test/.*' --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "[coverage]: analysing..."
        )
        message(STATUS "[coverage]: targeted >> [target=coverage]")
    endif ()
endfunction()

function(init_benchmark_data DATA_DIR)
    find_program(NODEJS_BINARY node)
    if (NODEJS_BINARY)
        __get_binary_version(
                ${NODEJS_BINARY}
                "--version"
                NODEJS_VERSION
        )
        message(STATUS "[nodejs]: found >> version=v${NODEJS_VERSION}, path='${NODEJS_BINARY}'")

        set(ROW_COUNT 100)
        set(SHORT_LOG 64)
        set(MEDIUM_LOG 256)
        set(LONG_LOG 1024)
        set(PLAIN 0)
        set(ESCAPED 10)
        set(HIGHLY_ESCAPED 25)
        set(BENCH_GEN_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/data-generator.js")

        if (NOT TARGET bench-prepare)
            add_custom_target(bench-prepare
                    COMMAND ${CMAKE_COMMAND} -E rm -rf "${DATA_DIR}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${DATA_DIR}"
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/short_plain_input.bin" ${ROW_COUNT} ${SHORT_LOG} ${PLAIN}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/medium_plain_input.bin" ${ROW_COUNT} ${MEDIUM_LOG} ${PLAIN}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/long_plain_input.bin" ${ROW_COUNT} ${LONG_LOG} ${PLAIN}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/short_escaped_input.bin" ${ROW_COUNT} ${SHORT_LOG} ${ESCAPED}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/medium_escaped_input.bin" ${ROW_COUNT} ${MEDIUM_LOG} ${ESCAPED}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/long_escaped_input.bin" ${ROW_COUNT} ${LONG_LOG} ${ESCAPED}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/short_highly_escaped_input.bin" ${ROW_COUNT} ${SHORT_LOG} ${HIGHLY_ESCAPED}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/medium_highly_escaped_input.bin" ${ROW_COUNT} ${MEDIUM_LOG} ${HIGHLY_ESCAPED}
                    COMMAND ${NODEJS_BINARY} "${BENCH_GEN_SCRIPT}"
                    "${DATA_DIR}/long_highly_escaped_input.bin" ${ROW_COUNT} ${LONG_LOG} ${HIGHLY_ESCAPED}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "[nodejs]: generating..."
            )
            message(STATUS "[nodejs]: targeted >> [target=bench-generate]")
        endif ()
    else ()
        if (NOT TARGET bench-prepare)
            add_custom_target(bench-prepare
                    COMMAND ${CMAKE_COMMAND} -E echo "[nodejs]: skipped >> [not found]"
                    COMMENT "[nodejs]: skipping..."
            )
        endif ()
        message(WARNING "[nodejs]: skipped >> [not found]")
    endif ()
endfunction()

function(init_bench SOURCE LIBRARIES)
    get_filename_component(raw_name ${SOURCE} NAME_WE)
    set(bench_target "bench_${raw_name}")
    add_executable(${bench_target} ${SOURCE})
    target_link_libraries(${bench_target} PRIVATE ${LIBRARIES})
    set_property(GLOBAL APPEND PROPERTY BENCH_TARGETS ${bench_target})
    message(STATUS "[bench]: ${bench_target}")
endfunction()

function(init_bench_run_all)
    get_property(ALL_BENCH_TARGETS GLOBAL PROPERTY BENCH_TARGETS)

    if (NOT ALL_BENCH_TARGETS)
        return()
    endif ()

    if (NOT TARGET bench-run)
        add_custom_target(bench-run
                COMMAND ${CMAKE_COMMAND} -E echo "Running all benchmarks..."
                COMMENT "Running all benchmarks..."
                USES_TERMINAL
        )
        foreach (BENCH_TARGET ${ALL_BENCH_TARGETS})
            add_custom_command(TARGET bench-run POST_BUILD
                    COMMAND $<TARGET_FILE:${BENCH_TARGET}>
                    COMMENT "[bench] Running: ${BENCH_TARGET}"
            )
        endforeach ()
        message(STATUS "[bench]: targeted >> [target=bench-run]")
    endif ()
endfunction()
