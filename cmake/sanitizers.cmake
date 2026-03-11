function(init_asan_flags TARGET)
    target_compile_options(${TARGET} INTERFACE
            -fsanitize=address
    )
    target_link_options(${TARGET} INTERFACE
            -fsanitize=address
            $<$<C_COMPILER_ID:GNU>: -static-libasan>
    )
    message(STATUS "[address-sanitizer]: enabled")
endfunction()

function(init_tsan_flags TARGET)
    target_compile_options(${TARGET} INTERFACE
            -fsanitize=thread
    )
    target_link_options(${TARGET} INTERFACE
            -fsanitize=thread
    )
    message(STATUS "[thread-sanitizer]: enabled")
endfunction()