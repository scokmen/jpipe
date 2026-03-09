#ifndef JPIPE_JP_COMMON_H
#define JPIPE_JP_COMMON_H

#include <stdio.h>

#define JP_PATH_MAX 4096
#define MIN(x, y)   (((x) < (y)) ? (x) : (y))

#ifdef __has_builtin
/**
 * @brief Compiler-agnostic check for built-in function support.
 *
 * Wraps the __has_builtin extension (available in Clang and GCC 10+) to
 * detect if the compiler provides a specific intrinsic/built-in function.
 * If the compiler is older and doesn't support detection, it safely
 * defaults to 0.
 *
 * @param built_in The built-in function name to check (e.g., __builtin_trap).
 * @return 1 if supported, 0 otherwise.
 */
#define HAS_BUILTIN(built_in) __has_builtin(built_in)
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define HAS_BUILTIN(built_in) (0)
#pragma message("cc: not supported (__has_builtin)")
#endif

#ifdef __has_attribute
/**
 * @brief Compiler-agnostic check for compiler attributes.
 *
 * Wraps the __has_attribute extension to detect if the compiler supports
 * specific __attribute__((...)) declarations. If the compiler does not
 * support attribute detection, it safely defaults to 0.
 *
 * Useful for applying optimizations or warnings (like packed structs,
 * visibility, or unused variables) only when the compiler supports them.
 *
 * @param attr The attribute name to check (e.g., format, aligned).
 * @return 1 if supported, 0 otherwise.
 */
#define HAS_ATTRIBUTE(attr) __has_attribute(attr)
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define HAS_ATTRIBUTE(attr) (0)
#pragma message("cc: not supported (__has_attribute)")
#endif

#if HAS_BUILTIN(__builtin_assume)
/**
 * @brief Provides a hint to the compiler that a condition is always true.
 *
 * This allows the optimizer to discard branches or simplify logic based on 'cond'.
 * 1. Uses __builtin_assume if available (Clang).
 * 2. Falls back to __builtin_unreachable() (GCC), signaling that the false branch
 * can never be taken.
 * 3. Defaults to a no-op if no compiler support is found.
 *
 * @warning Use with extreme caution. If 'cond' is ever false at runtime,
 * the program results in Undefined Behavior (UB).
 *
 * @param cond The boolean condition assumed to be true.
 */
#define JP_ASSUME(cond) __builtin_assume(cond)
#elif HAS_BUILTIN(__builtin_unreachable)
/**
 * @brief Provides a hint to the compiler that a condition is always true.
 *
 * This allows the optimizer to discard branches or simplify logic based on 'cond'.
 * 1. Uses __builtin_assume if available (Clang).
 * 2. Falls back to __builtin_unreachable() (GCC), signaling that the false branch
 * can never be taken.
 * 3. Defaults to a no-op if no compiler support is found.
 *
 * @warning Use with extreme caution. If 'cond' is ever false at runtime,
 * the program results in Undefined Behavior (UB).
 *
 * @param cond The boolean condition assumed to be true.
 */
#define JP_ASSUME(cond)              \
    do {                             \
        if (!(cond))                 \
            __builtin_unreachable(); \
    } while (0)
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_ASSUME(cond) ((void) 0)
#pragma message("cc: not supported (__builtin_assume || __builtin_unreachable)")
#endif

#if HAS_BUILTIN(__builtin_expect)
/**
 * @brief Optimization hint for the "Happy Path" (most likely execution path).
 *
 * Instructs the compiler to prioritize the branch where 'cond' is true.
 * This keeps the instruction pipeline full for the common case,
 * minimizing branch misprediction penalties.
 *
 * @param cond Condition expected to be true.
 */
#define JP_LIKELY(cond)   __builtin_expect(!!(cond), 1)
/**
 * @brief Optimization hint for the "Error Path" (least likely execution path).
 *
 * Tells the compiler that 'cond' is rarely true. The optimizer may move the
 * code inside this branch to a "cold" section of the binary, improving
 * instruction cache locality for the hot (likely) code.
 *
 * @param x Condition expected to be false.
 */
#define JP_UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_LIKELY(cond)   (cond)
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_UNLIKELY(cond) (cond)
#pragma message("cc: not supported (__builtin_expect)")
#endif

#if HAS_ATTRIBUTE(malloc)
/**
 * @brief Hints that a function returns a newly allocated pointer.
 *
 * This attribute tells the compiler that the returned pointer cannot "alias"
 * any other existing pointer. It allows the optimizer to assume that memory
 * pointed to by the return value is distinct from any other accessible memory.
 *
 * This leads to significant optimizations in loop unrolling and instruction
 * scheduling because the compiler knows that writing to this new memory
 * won't affect other variables.
 *
 * @note Best used for custom allocator wrappers or factory functions.
 */
#define JP_MALLOC __attribute__((malloc))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_MALLOC
#pragma message("cc: attribute not supported (malloc)")
#endif

#if HAS_ATTRIBUTE(warn_unused_result)
/**
 * @brief Enforces that the return value of a function must be checked.
 *
 * If a function decorated with this macro is called and its return value
 * is discarded, the compiler will issue a warning (or an error with -Werror).
 * This is critical for functions returning error codes or allocated resources
 * that must be managed.
 */
#define JP_USE_RESULT __attribute__((warn_unused_result))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_USE_RESULT
#pragma message("cc: attribute not supported (warn_unused_result)")
#endif

#if HAS_ATTRIBUTE(const)
/**
 * @brief Specifies that a function has no side effects and depends only on its arguments.
 *
 * A "const" function does not examine any values except its arguments, and have
 * no effects except its return value. It cannot read global memory or
 * call non-const functions.
 *
 * This allows the compiler to:
 * 1. Perform Common Subexpression Elimination (CSE).
 * 2. Cache the result if the function is called multiple times with same arguments.
 * 3. Delete the call entirely if the return value is not used.
 *
 * @warning Do not use this if the function reads pointers, global variables,
 * or volatile memory.
 */
#define JP_CONST_FUNC __attribute__((const))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_CONST_FUNC
#pragma message("cc: attribute not supported (const)")
#endif

#if HAS_ATTRIBUTE(format)
/**
 * @brief Enables printf-style format string validation for custom functions.
 *
 * This attribute instructs the compiler to check the arguments passed to the
 * function against the format string, just like it does for printf().
 * It helps catch type mismatches and missing arguments at compile time.
 *
 * @param fmt The index of the format string argument (1-based).
 * The arguments to check are assumed to start at (fmt + 1).
 */
#define JP_PRINT_FUNC(fmt) __attribute__((format(printf, fmt, (fmt) + 1)))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_PRINT_FUNC
#pragma message("cc: attribute not supported (format)")
#endif

#if HAS_ATTRIBUTE(unused)
/**
 * @brief Marks a variable, function, or parameter as intentionally unused.
 *
 * Prevents the compiler from issuing "-Wunused-variable" or "-Wunused-parameter"
 * warnings. This is particularly useful for:
 * 1. Parameters in callback functions that are required by the API but not needed.
 * 2. Variables used only in debug assertions (assert()).
 * 3. Stub functions intended for future use.
 */
#define JP_UNUSED __attribute__((unused))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_UNUSED
#pragma message("cc: attribute not supported (unused)")
#endif

#if HAS_ATTRIBUTE(nonnull)
/**
 * @brief Specifies that certain pointer arguments must not be NULL.
 *
 * This attribute allows the compiler to:
 * 1. Issue a warning at compile-time if it detects a NULL value being passed.
 * 2. Optimize out NULL checks inside the function, assuming the caller
 * has followed the contract.
 *
 * @param ... The indices of the arguments that are required to be non-null (1-based).
 * If no indices are provided, all pointer arguments are marked non-null.
 */
#define JP_NONNULL_ARG(...) __attribute__((nonnull(__VA_ARGS__)))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_NONNULL_ARG(...)
#pragma message("cc: attribute not supported (nonnull)")
#endif

#if HAS_ATTRIBUTE(access)
/**
 * @brief Informs the compiler that a pointer argument is only used for reading.
 *
 * This attribute enables stronger static analysis by the compiler's
 * -Wstringop-overflow and analyzer passes. It guarantees that the function
 * will not modify the memory pointed to by 'ptr_idx'.
 *
 * It is a more powerful version of 'const' for the compiler, as it defines
 * the specific access intent at the architectural level.
 *
 * @param ptr_idx The 1-based index of the pointer argument.
 */
#define JP_READ_PTR(ptr_idx)                 __attribute__((access(read_only, ptr_idx)))
/**
 * @brief Informs the compiler that a pointer is read-only and defines its bound.
 *
 * Tells the optimizer and static analyzer that the function reads 'size_idx'
 * elements from the pointer at 'ptr_idx'. This allows the compiler to detect
 * out-of-bounds reads at compile-time.
 *
 * @param ptr_idx  The 1-based index of the pointer argument.
 * @param size_idx The 1-based index of the argument representing the buffer size.
 */
#define JP_READ_PTR_SIZE(ptr_idx, size_idx)  __attribute__((access(read_only, ptr_idx, size_idx)))
/**
 * @brief Informs the compiler that a pointer argument is used for writing.
 *
 * This attribute hints that the function will modify the memory pointed to
 * by 'ptr_idx'. It helps the compiler's static analyzer detect if
 * uninitialized memory is being passed to a function that expects to
 * write to it, or if a constant pointer is incorrectly passed.
 *
 * Useful for functions that return results via pointer arguments.
 *
 * @param ptr_idx The 1-based index of the pointer argument.
 */
#define JP_WRITE_PTR(ptr_idx)                __attribute__((access(write_only, ptr_idx)))
/**
 * @brief Informs the compiler that a pointer is write-only and defines its bound.
 *
 * This attribute allows the compiler to verify that the buffer pointed to by
 * 'ptr_idx' is large enough to handle 'size_idx' amount of data. It is
 * essential for preventing buffer overflows during output operations.
 *
 * @param ptr_idx  The 1-based index of the pointer argument.
 * @param size_idx The 1-based index of the argument representing the buffer size.
 */
#define JP_WRITE_PTR_SIZE(ptr_idx, size_idx) __attribute__((access(write_only, ptr_idx, size_idx)))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_READ_PTR(ptr_idx)
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_READ_PTR_SIZE(ptr_idx, size_idx)
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_WRITE_PTR(ptr_idx)
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_WRITE_PTR_SIZE(ptr_idx, size_idx)
#pragma message("cc: attribute not supported (access)")
#endif

#if HAS_ATTRIBUTE(fd_arg)
/**
 * @brief Marks an integer argument as a file descriptor.
 *
 * This attribute allows the compiler's static analyzer to track the lifecycle
 * of file descriptors. It helps detect common bugs such as:
 * 1. Passing an invalid or closed file descriptor.
 * 2. Performing I/O operations on a file descriptor that wasn't properly opened.
 * 3. Resource leaks (forgetting to close a descriptor).
 *
 * @param n The 1-based index of the file descriptor argument.
 */
#define JP_FILE_DESC(n) __attribute__((fd_arg(n)))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_FILE_DESC(n)
#pragma message("cc: attribute not supported (fd_arg)")
#endif

#if HAS_ATTRIBUTE(fd_arg_read)
/**
 * @brief Marks an integer argument as a file descriptor opened for reading.
 *
 * This is a specialized version of JP_FILE_DESC. It informs the static
 * analyzer that the function expects a file descriptor with read permissions.
 * It helps catch logic errors where a write-only descriptor (e.g., opened with O_WRONLY)
 * is passed to a function intended for data input.
 *
 * @param n The 1-based index of the file descriptor argument.
 */
#define JP_FILE_DESC_READ(n) __attribute__((fd_arg_read(n)))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_FILE_DESC_READ(n)
#pragma message("cc: attribute not supported (fd_arg_read)")
#endif

#if HAS_ATTRIBUTE(fallthrough)
/**
 * @brief Indicates that a switch case fall-through is intentional.
 *
 * Prevents the compiler from issuing a warning when a 'case' block does
 * not end with a 'break' or 'return'. This clarifies intent to both
 * the compiler's static analyzer and other developers.
 */
#define JP_FALLTHROUGH __attribute__((fallthrough))
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_FALLTHROUGH
#pragma message("cc: attribute not supported (fallthrough)")
#endif

/**
 * @brief Safely frees a pointer and sets it to NULL.
 *
 * Checks if the pointer is not NULL before calling free(). After freeing,
 * it explicitly sets the pointer to NULL to prevent dangling pointer issues
 * and accidental double-free errors.
 *
 * @note The cast to (void*) ensures compatibility with various pointer types
 * and avoids potential warnings with 'const' pointers.
 *
 * @param ptr The pointer to be freed (must be an lvalue).
 */
#define JP_FREE(ptr)             \
    do {                         \
        if ((ptr) != NULL) {     \
            free((void*) (ptr)); \
            (ptr) = NULL;        \
        }                        \
    } while (0)

/**
 * @brief Safely allocates memory/resource and returns on failure.
 * * Assigns 'expr' to 'var'. If the assignment fails (NULL), it executes
 * 'return ret'. Uses do-while(0) for statement safety and JP_UNLIKELY
 * for branch optimization.
 *
 * @param var  Target variable to store the allocation result.
 * @param expr The allocation expression (e.g., malloc, strdup).
 * @param ret  The value to return upon failure.
 */
#define JP_ALLOC_OR_RET(var, expr, ret)   \
    do {                                  \
        (var) = (expr);                   \
        if (JP_UNLIKELY((var) == NULL)) { \
            return (ret);                 \
        }                                 \
    } while (0)

/**
 * @brief Executes a statement and returns its error code if it is non-zero.
 *
 * This macro captures the result of 'stm' into a temporary variable to avoid
 * double evaluation. If the result is not 0 (standard for errors in POSIX/C),
 * it immediately returns that value from the current function.
 *
 * @note Uses __typeof__ (GCC/Clang extension) for type safety and to support
 * various return types (int, long, etc.) without explicit casting.
 *
 * @param stm The expression or function call to evaluate.
 */
#define JP_OK_OR_RET(stm)              \
    do {                               \
        __typeof__(stm) __err = (stm); \
        if (__err != 0) {              \
            return __err;              \
        }                              \
    } while (0)

/**
 * @brief Standard logging macro for general output.
 *
 * It appends a newline to every message for consistent formatting.
 * It uses the GNU '##' extension to safely handle cases where no variadic
 * arguments are provided, preventing trailing comma compilation errors.
 *
 * @param fmt Format string (printf-style).
 * @param ... Variadic arguments for the format string.
 */
#define JP_LOG(fmt, ...) fprintf(stdout, fmt "\n", ##__VA_ARGS__)

#ifndef NDEBUG
/**
 * @brief Conditionally prints debug information to stdout.
 *
 * When NDEBUG is not defined, this macro behaves like fprintf, adding a "[DEBUG]" prefix and a newline.
 * When NDEBUG is defined (Release mode), it evaluates to a no-op, ensuring zero runtime overhead and
 * removing debug strings from the binary.
 *
 * @param fmt Format string (printf-style).
 * @param ... Variadic arguments for the format string.
 */
#define JP_DEBUG(fmt, ...) fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
/**
 * @brief This macro is currently disabled because the compiler/toolchain does not support the required attribute.
 */
#define JP_DEBUG(fmt, ...) ((void) 0)
#endif

#endif  // JPIPE_JP_COMMON_H
