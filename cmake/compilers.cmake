function(init_compiler_flags TARGET)
    set(COMPILE_FLAGS "")

    # TODO: Add GCC version specific compiler flags.
    # TODO: Add distribution target specific compiler flags.

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND COMPILE_FLAGS
                -g
                -O1
                -fno-omit-frame-pointer
        )
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND COMPILE_FLAGS
                -O3
                -flto
                -fomit-frame-pointer
                -march=native
                -DNDEBUG
                -DNMOCK
        )
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        list(APPEND COMPILE_FLAGS
                -g
                -O2
                -fno-omit-frame-pointer
                -march=native
                -DNDEBUG
        )
    endif ()

    list(APPEND COMPILE_FLAGS
            -Wall
            -Wextra
            -Wconversion
            -Wshadow
            -Wstrict-prototypes
            -Wwrite-strings
            -Wnull-dereference
            -Wdouble-promotion
            -Werror
            -Wformat=2
            -Wno-gnu-zero-variadic-macro-arguments
            -Wduplicated-branches
            -Wduplicated-cond
            -Wlogical-op
            -Wrestrict
            -fanalyzer
            -Wanalyzer-file-leak
            -Wanalyzer-use-after-free
    )

    target_supported_headers(${TARGET} INTERFACE "${COMPILE_FLAGS}")
endfunction()