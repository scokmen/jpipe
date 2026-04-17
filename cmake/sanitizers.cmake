function(init_asan_flags TARGET)
    check_flag_supported("-fsanitize=address" IS_ASAN_SUPPORTED)
    if (NOT IS_ASAN_SUPPORTED)
        message(FATAL_ERROR "[cc][asan]: The Address Sanitizer is not supported on this platform. Please consider not using -DENABLE_ASAN=ON option.")
        return()
    endif ()
    
    target_compile_options(${TARGET} INTERFACE
            -fsanitize=address
    )
    target_link_options(${TARGET} INTERFACE
            -fsanitize=address
            $<$<C_COMPILER_ID:GNU>: -static-libasan>
    )
    message(STATUS "[cc][asan]: Enabled.")
endfunction()

function(init_tsan_flags TARGET)
    check_flag_supported("-fsanitize=thread" IS_TSAN_SUPPORTED)
    if (NOT IS_TSAN_SUPPORTED)
        message(FATAL_ERROR "[cc][tsan]: The Thread Sanitizer is not supported on this platform. Please consider not using -DENABLE_TSAN=ON option.")
        return()
    endif ()
    
    target_compile_options(${TARGET} INTERFACE
            -fsanitize=thread
    )
    target_link_options(${TARGET} INTERFACE
            -fsanitize=thread
    )
    message(STATUS "[cc][tsan]: Enabled.")
endfunction()