###############################################
# JPipe: Core Library
###############################################
set(NON_PORTABLE_C_FILES "")

if (FLAG_EPOLL)
    message(STATUS "[polling]: sys/epoll.h")
    list(APPEND NON_PORTABLE_C_FILES "poller/epoll.c")
elseif (FLAG_KQUEUE)
    message(STATUS "[polling]: sys/event.h")
    list(APPEND NON_PORTABLE_C_FILES "poller/kqueue.c")
else ()
    message(FATAL_ERROR "[polling]: Unsupported polling platform.")
endif ()

add_library(jpipe_core STATIC
        worker.c
        queue.c
        command.c
        field.c
        errno.c
        reader.c
        writer.c
        config.c
        ${NON_PORTABLE_C_FILES}
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
# JPipe: Clang-Tidy & Clang-Format
###############################################
init_clang_format()
init_cppcheck()
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