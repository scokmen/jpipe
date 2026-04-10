function(init_compiler_flags TARGET)
    # TODO: Add distribution target specific compiler flags.
    target_compile_options(${TARGET} INTERFACE
            $<$<CONFIG:Debug>: -g -O1 -fno-omit-frame-pointer>
            $<$<CONFIG:Release>: -O3 -flto -fomit-frame-pointer -march=native -DNDEBUG>
            $<$<CONFIG:RelWithDebInfo>: -g -O2 -fno-omit-frame-pointer -march=native -DNDEBUG>
    )

    set(SHARED_FLAGS
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
    )

    set(CLANG_FLAGS
            $<$<C_COMPILER_ID:Clang>:
            -Wno-gnu-zero-variadic-macro-arguments>
    )

    # TODO: Add GCC version specific compiler flags.
    set(GCC_FLAGS
            $<$<C_COMPILER_ID:GNU>:
            -Wduplicated-branches
            -Wduplicated-cond
            -Wlogical-op
            -Wrestrict
            -fanalyzer
            -Wanalyzer-file-leak
            -Wanalyzer-use-after-free
            >
    )

    target_compile_options(${TARGET} INTERFACE
            ${SHARED_FLAGS}
            ${CLANG_FLAGS}
            ${GCC_FLAGS}
    )
endfunction()
