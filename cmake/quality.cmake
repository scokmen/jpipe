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
        message(STATUS "[cq][clang-tidy]: Skipped. ClangTidy is not supported on this platform ${CMAKE_C_COMPILER_ID}.")
        return()
    endif ()

    find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
    if (CLANG_TIDY_BINARY)
        __get_binary_version(
                ${CLANG_TIDY_BINARY}
                "--version"
                CLANG_TIDY_VERSION
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
        message(STATUS "[cq][clang-tidy]: Enabled (version=${CLANG_TIDY_VERSION}, path=${CLANG_TIDY_BINARY}).")
    else ()
        message(STATUS "[cq][clang-tidy]: Skipped. Binary cannot be found.")
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
                    COMMENT "[cq][clang-format]: Running..."
            )
            message(STATUS "[cq][clang-format]: Targeted (target=format, version=v${CLANG_FORMAT_VERSION}, path=${CLANG_FORMAT_BINARY}).")
        endif ()
    else ()
        if (NOT TARGET format)
            add_custom_target(format
                    COMMAND ${CMAKE_COMMAND} -E echo "[cq][clang-format]: Failed. Binary cannot be found."
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "[cq][clang-format]: Running..."
            )
        endif ()
        message(WARNING "[cq][clang-format]: Failed. Binary cannot be found. Please consider not using target=format command.")
    endif ()
endfunction()

function(init_cppcheck)
    find_program(CPPCHECK_BINARY "cppcheck")
    if (CPPCHECK_BINARY)
        __get_binary_version(
                ${CPPCHECK_BINARY}
                "--version"
                CPPCHECK_VERSION
        )

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
                    COMMENT "[cq][cppcheck/inform]: Running..."
            )
            message(STATUS "[cq][cppcheck/inform]: Targeted (target=check-and-inform, version=v${CPPCHECK_VERSION}, path=${CPPCHECK_BINARY}).")
        endif ()
        if (NOT TARGET check-and-enforce)
            add_custom_target(check-and-enforce
                    COMMAND ${CPPCHECK_BINARY}
                    ${CPPCHECK_BASE_FLAGS}
                    --enable=warning,performance,portability
                    --error-exitcode=1
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMENT "[cq][cppcheck/enforce]: Running..."
            )
            message(STATUS "[cq][cppcheck/enforce]: Targeted (target=check-and-enforce, version=v${CPPCHECK_VERSION}, path=${CPPCHECK_BINARY}).")
        endif ()
    else ()
        if (NOT TARGET check-and-inform)
            add_custom_target(check-and-inform
                    COMMAND ${CMAKE_COMMAND} -E echo "[cq][cppcheck/inform]: Failed. Binary cannot be found."
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "[cq][cppcheck/inform]: Running..."
            )
        endif ()
        message(WARNING "[cq][cppcheck/inform]: Failed. Binary cannot be found. Please consider not using target=check-and-inform command.")
        if (NOT TARGET check-and-enforce)
            add_custom_target(check-and-enforce
                    COMMAND ${CMAKE_COMMAND} -E echo "[cq][cppcheck/enforce]: Failed. Binary cannot be found."
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "[cq][cppcheck/enforce]: Running..."
            )
        endif ()
        message(WARNING "[cq][cppcheck/enforce]: Failed. Binary cannot be found. Please consider not using target=check-and-enforce command.")
    endif ()
endfunction()

function(init_coverage_flags TARGET)
    check_flag_supported("--coverage" IS_COV_SUPPORTED)
    if (NOT IS_COV_SUPPORTED)
        message(WARNING "[cq][coverage][flag]: Failed. Coverage flag is not supported on this platform. Please consider not using target=coverage command.")
    endif ()

    find_program(LCOV_BINARY NAMES "lcov")
    if (LCOV_BINARY)
        __get_binary_version(
                ${LCOV_BINARY}
                "--version"
                LCOV_VERSION
        )
        message(STATUS "[cq][coverage][lcov]: Enabled (version=v${LCOV_VERSION}, path=${LCOV_BINARY}).")
    else ()
        message(WARNING "[cq][coverage][lcov]: Failed. Binary cannot be found. Please consider not using target=coverage command.")
    endif ()

    find_program(GCOVR_BINARY NAMES "gcovr")
    if (GCOVR_BINARY)
        __get_binary_version(
                ${GCOVR_BINARY}
                "--version"
                GCOVR_VERSION
        )
        message(STATUS "[cq][coverage][gcovr]: Enabled (version=v${GCOVR_VERSION}, path=${GCOVR_BINARY}).")
    else ()
        message(WARNING "[cq][coverage][gcovr]: Failed. Binary cannot be found. Please consider not using target=coverage command.")
    endif ()

    if (NOT (LCOV_BINARY AND GCOVR_BINARY AND IS_COV_SUPPORTED))
        if (NOT TARGET coverage)
            add_custom_target(coverage
                    COMMAND ${CMAKE_COMMAND} -E echo "[cq][coverage]: Failed. Prerequisites are failed."
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "[cq][coverage]: Running..."
            )
        endif ()
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
                COMMENT "[cq][coverage]: Running..."
        )
        message(STATUS "[cq][coverage]: Targeted (target=coverage).")
    endif ()
endfunction()

function(init_test LABEL SOURCE LIBRARIES)
    get_filename_component(raw_name ${SOURCE} NAME_WE)
    set(test_target "${LABEL}_test_${raw_name}")
    add_executable(${test_target} ${SOURCE})
    target_link_libraries(${test_target} PRIVATE ${LIBRARIES})
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES LABELS ${LABEL})
    message(STATUS "[cq][test]: ${test_target}")
endfunction()

function(target_benchmark_prepare_command DATA_DIR)
    find_program(NODEJS_BINARY "node")
    if (NODEJS_BINARY)
        __get_binary_version(
                ${NODEJS_BINARY}
                "--version"
                NODEJS_VERSION
        )

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
                    COMMENT "[cq][bench][data]: Running..."
            )
            message(STATUS "[cq][bench][data]: Targeted (target=bench-prepare, version=v${NODEJS_VERSION}, path=${NODEJS_BINARY}).")
        endif ()
    else ()
        if (NOT TARGET bench-prepare)
            add_custom_target(bench-prepare
                    COMMAND ${CMAKE_COMMAND} -E echo "[cq][bench][data]: Failed. Binary cannot be found."
                    COMMAND ${CMAKE_COMMAND} -E false
                    COMMENT "[cq][bench][data]: Running..."
            )
        endif ()
        message(WARNING "[cq][bench][data]: Failed. Binary cannot be found. Please consider not using target=bench-prepare command.")
    endif ()
endfunction()

function(init_bench SOURCE LIBRARIES)
    get_filename_component(raw_name ${SOURCE} NAME_WE)
    set(bench_target "bench_${raw_name}")
    add_executable(${bench_target} ${SOURCE})
    target_link_libraries(${bench_target} PRIVATE ${LIBRARIES})
    set_property(GLOBAL APPEND PROPERTY BENCH_TARGETS ${bench_target})
    message(STATUS "[cq][bench]: ${bench_target}")
endfunction()

function(target_benchmark_run_command)
    get_property(ALL_BENCH_TARGETS GLOBAL PROPERTY BENCH_TARGETS)
    
    if (NOT TARGET bench-run)
        add_custom_target(bench-run
                COMMAND ${CMAKE_COMMAND} -E echo "[cq][bench]: Running..."
                COMMENT "[cq][bench]: Running..."
                USES_TERMINAL
        )
        foreach (BENCH_TARGET ${ALL_BENCH_TARGETS})
            add_custom_command(TARGET bench-run POST_BUILD
                    COMMAND $<TARGET_FILE:${BENCH_TARGET}>
                    COMMENT "[cq][bench]: Running: ${BENCH_TARGET}"
            )
        endforeach ()
        message(STATUS "[cq][bench]: Targeted (target=bench-run)")
    endif ()
endfunction()
