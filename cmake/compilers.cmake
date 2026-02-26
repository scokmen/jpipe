function(init_debug_flags TARGET)
    target_compile_options(${TARGET} INTERFACE
            -g
            -O1
    )
endfunction()

function(init_release_flags TARGET)
    # TODO: Add a "march" flag for the distribution targets.
    target_compile_options(${TARGET} INTERFACE
            -O3
            -flto
            -fomit-frame-pointer
    )
    target_compile_definitions(${TARGET} INTERFACE
            NDEBUG
    )
endfunction()

function(init_compiler_flags TARGET)
    target_compile_options(${TARGET} INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wstrict-prototypes
            -Wwrite-strings
            -Wnull-dereference
            -Wdouble-promotion
            -Werror
            -Wformat=2
            -fno-omit-frame-pointer
    )

    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${TARGET} INTERFACE
                -Wno-gnu-zero-variadic-macro-arguments
        )
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${TARGET} INTERFACE
                -Wduplicated-branches
                -Wduplicated-cond
                -Wlogical-op
                -Wrestrict
        )
    endif ()
endfunction()