###############################################
# JPipe: Test Utils
###############################################
add_library(test_utils STATIC
        lib/jp_test.c
)

target_include_directories(test_utils PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/generated"
)

target_compile_definitions(test_utils PUBLIC
        JP_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

target_link_libraries(test_utils PRIVATE
        jpipe_settings
)

###############################################
# JPipe: Unit Tests
###############################################
set(UNIT_TEST_SOURCES
        unit/jp_field_test.c
        unit/jp_command_test.c
        unit/jp_worker_test.c
        unit/jp_queue_test.c
)


foreach (source_file ${UNIT_TEST_SOURCES})
    get_filename_component(raw_name ${source_file} NAME_WE)
    set(test_target "unit_test_${raw_name}")
    add_executable(${test_target} ${source_file})
    target_link_libraries(${test_target}
            PRIVATE
            jpipe_core
            test_utils
            jpipe_settings
    )
    target_include_directories(${test_target} PRIVATE "${CMAKE_BINARY_DIR}/generated")
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES LABELS "unit")
endforeach ()

###############################################
# JPipe: Stress Tests
###############################################
set(STRESS_TEST_SOURCES
        stress/jp_queue_test.c
)

foreach (source_file ${STRESS_TEST_SOURCES})
    get_filename_component(raw_name ${source_file} NAME_WE)
    set(test_target "stress_test_${raw_name}")
    add_executable(${test_target} ${source_file})
    target_link_libraries(${test_target}
            PRIVATE
            jpipe_core
            test_utils
            jpipe_settings
    )
    target_include_directories(${test_target} PRIVATE "${CMAKE_BINARY_DIR}/generated")
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES LABELS "stress")
endforeach ()