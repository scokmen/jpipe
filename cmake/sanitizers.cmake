function(init_asan_flags TARGET)
    target_compile_options(${TARGET} INTERFACE
            -fsanitize=address
    )

    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_link_libraries(${TARGET} INTERFACE
                -fsanitize=address
        )
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_options(${TARGET} INTERFACE
                -fsanitize=address
                -static-libasan
        )
    endif ()
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