###############################################
# JPipe: Test & Coverage
###############################################
set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

init_coverage_flags(jpipe_settings)

add_library(test_utils STATIC
        lib/test.c
)

target_include_directories(test_utils PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/generated"
)

target_compile_definitions(test_utils PUBLIC
        JP_TEST_DATA_DIR="${TEST_DATA_DIR}"
)

target_link_libraries(test_utils PRIVATE
        jpipe_settings
)

SET(TEST_DEPENDENCIES
        jpipe_core
        test_utils
        jpipe_settings)

init_test("unit" unit/jp_field_test.c "${TEST_DEPENDENCIES}")
init_test("unit" unit/jp_command_test.c "${TEST_DEPENDENCIES}")
init_test("unit" unit/jp_queue_test.c "${TEST_DEPENDENCIES}")
init_test("unit" unit/jp_reader_test.c "${TEST_DEPENDENCIES}")
init_test("unit" unit/jp_writer_test.c "${TEST_DEPENDENCIES}")
init_test("unit" unit/jp_worker_test.c "${TEST_DEPENDENCIES}")
init_test("unit" unit/jp_json_test.c "${TEST_DEPENDENCIES}")
init_test("stress" stress/jp_queue_test.c "${TEST_DEPENDENCIES}")
init_test("stress" stress/jp_reader_test.c "${TEST_DEPENDENCIES}")
init_test("stress" stress/jp_writer_test.c "${TEST_DEPENDENCIES}")
