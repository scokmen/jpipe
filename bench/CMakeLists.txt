###############################################
# JPipe: Micro Benchmarking
###############################################
set(BENCH_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")

target_benchmark_prepare_command("${BENCH_DATA_DIR}")

add_library(bench_utils STATIC
        lib/bench.c
)

target_include_directories(bench_utils PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/generated"
)

target_compile_definitions(bench_utils PUBLIC
        JP_BENCH_DATA_DIR="${BENCH_DATA_DIR}"
)

target_link_libraries(bench_utils PRIVATE
        jpipe_settings
)

SET(BENCH_DEPENDENCIES
        jpipe_core
        bench_utils
        jpipe_settings)

init_bench(json_encoding.c "${BENCH_DEPENDENCIES}")

target_benchmark_run_command()
