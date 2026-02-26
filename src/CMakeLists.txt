###############################################
# JPipe: Core Library
###############################################
add_library(jpipe_core STATIC
        worker.c
        queue.c
        command.c
        field.c
        errno.c
)

target_include_directories(jpipe_core
        PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/generated"
)

target_link_libraries(jpipe_core
        PUBLIC Threads::Threads
        PRIVATE jpipe_settings
)

###############################################
# JPipe: Clang-Tidy >> Core
###############################################
init_clang_tidy(jpipe_core)

###############################################
# JPipe: Main Executable
###############################################
add_executable(jpipe main.c)

target_link_libraries(jpipe
        PRIVATE
        jpipe_core
        jpipe_settings
)