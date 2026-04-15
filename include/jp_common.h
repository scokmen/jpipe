#ifndef JPIPE_JP_COMMON_H
#define JPIPE_JP_COMMON_H

#include <jp_config.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef __has_builtin
/**
 * @brief Compiler-agnostic check for built-in function support.
 *
 * Wraps the __has_builtin extension (available in Clang and GCC 10+) to
 * detect if the compiler provides a specific intrinsic/built-in function.
 * If the compiler is older and doesn't support detection, it safely defaults to 0.
 *
 * @param built_in The built-in function name to check (e.g., __builtin_trap).
 * @return 1 if supported, 0 otherwise.
 */
#define HAS_BUILTIN(built_in) __has_builtin(built_in)
#else
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
#define HAS_ATTRIBUTE(attr) (0)
#pragma message("cc: not supported (__has_attribute)")
#endif

#if HAS_BUILTIN(__builtin_assume)
/**
 * @brief Provides a hint to the compiler that a condition is always true.
 *
 * This allows the optimizer to discard branches or simplify logic based on 'cond'.
 * 1. Uses __builtin_assume if available (Clang).
 * 2. Falls back to __builtin_unreachable() (GCC), signaling that the false branch can never be taken.
 * 3. Defaults to a no-op if no compiler support is found.
 *
 * @warning Use with extreme caution. If 'cond' is ever false at runtime,
 * the program results in Undefined Behavior (UB).
 *
 * @param cond The boolean condition assumed to be true.
 */
#define JP_ATTR_ASSUME(cond) __builtin_assume(cond)
#elif HAS_BUILTIN(__builtin_unreachable)
/**
 * @brief Provides a hint to the compiler that a condition is always true.
 *
 * This allows the optimizer to discard branches or simplify logic based on 'cond'.
 * 1. Uses __builtin_assume if available (Clang).
 * 2. Falls back to __builtin_unreachable() (GCC), signaling that the false branch can never be taken.
 * 3. Defaults to a no-op if no compiler support is found.
 *
 * @warning Use with extreme caution. If 'cond' is ever false at runtime,
 * the program results in Undefined Behavior (UB).
 *
 * @param cond The boolean condition assumed to be true.
 */
#define JP_ATTR_ASSUME(cond)         \
    do {                             \
        if (!(cond))                 \
            __builtin_unreachable(); \
    } while (0)
#else
#define JP_ATTR_ASSUME(cond) ((void) 0)
#pragma message("cc: not supported (__builtin_assume || __builtin_unreachable)")
#endif

#if HAS_BUILTIN(__builtin_expect)
/**
 * @brief Optimization hint for the "Happy Path" (most likely execution path).
 *
 * Instructs the compiler to prioritize the branch where 'cond' is true.
 * This keeps the instruction pipeline full for the common case, minimizing branch misprediction penalties.
 *
 * @param cond Condition expected to be true.
 */
#define JP_ATTR_LIKELY(cond)   __builtin_expect(!!(cond), 1)
/**
 * @brief Optimization hint for the "Error Path" (least likely execution path).
 *
 * Tells the compiler that 'cond' is rarely true.
 * The optimizer may move the code inside this branch to a "cold" section of the binary,
 * improving instruction cache locality for the hot (likely) code.
 *
 * @param cond Condition expected to be false.
 */
#define JP_ATTR_UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#else
#define JP_ATTR_LIKELY(cond)   (cond)
#define JP_ATTR_UNLIKELY(cond) (cond)
#pragma message("cc: not supported (__builtin_expect)")
#endif

#if HAS_ATTRIBUTE(malloc)
/**
 * @brief Hints that a function returns a newly allocated pointer.
 *
 * This attribute tells the compiler that the returned pointer cannot "alias" any other existing pointer.
 * It allows the optimizer to assume that memory pointed to by the return value is distinct from any other accessible
 * memory.
 *
 * This leads to significant optimizations in loop unrolling and instruction scheduling because the compiler
 * knows that writing to this new memory won't affect other variables.
 *
 * @note Best used for custom allocator wrappers or factory functions.
 */
#define JP_ATTR_ALLOCATED __attribute__((malloc))
#else
#define JP_ATTR_ALLOCATED
#pragma message("cc: attribute not supported (malloc)")
#endif

#if HAS_ATTRIBUTE(warn_unused_result)
/**
 * @brief Enforces that the return value of a function must be checked.
 *
 * If a function decorated with this macro is called and its return value is discarded,
 * the compiler will issue a warning (or an error with -Werror).
 * This is critical for functions returning error codes or allocated resources that must be managed.
 */
#define JP_ATTR_USE_RETURN __attribute__((warn_unused_result))
#else
#define JP_ATTR_USE_RETURN
#pragma message("cc: attribute not supported (warn_unused_result)")
#endif

#if HAS_ATTRIBUTE(const)
/**
 * @brief Specifies that a function has no side effects and depends only on its arguments.
 *
 * A "const" function does not examine any values except its arguments, and have no effects except its return value.
 * It cannot read global memory or call non-const functions.
 *
 * This allows the compiler to:
 * 1. Perform Common Subexpression Elimination (CSE).
 * 2. Cache the result if the function is called multiple times with same arguments.
 * 3. Delete the call entirely if the return value is not used.
 *
 * @warning Do not use this if the function reads pointers, global variables, or volatile memory.
 */
#define JP_ATTR_CONST __attribute__((const))
#else
#define JP_ATTR_CONST
#pragma message("cc: attribute not supported (const)")
#endif

#if HAS_ATTRIBUTE(format)
/**
 * @brief Enables printf-style format string validation for custom functions.
 *
 * This attribute instructs the compiler to check the arguments passed to the function against the format string,
 * just like it does for printf(). It helps catch type mismatches and missing arguments at compile time.
 *
 * @param fmt The index of the format string argument (1-based).
 * The arguments to check are assumed to start at (fmt + 1).
 */
#define JP_ATTR_FORMAT(fmt) __attribute__((format(printf, fmt, (fmt) + 1)))
#else
#define JP_ATTR_FORMAT(fmt)
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
#define JP_ATTR_UNUSED __attribute__((unused))
#else
#define JP_ATTR_UNUSED
#pragma message("cc: attribute not supported (unused)")
#endif

#if HAS_ATTRIBUTE(nonnull)
/**
 * @brief Specifies that certain pointer arguments must not be NULL.
 *
 * This attribute allows the compiler to:
 * 1. Issue a warning at compile-time if it detects a NULL value being passed.
 * 2. Optimize out NULL checks inside the function, assuming the caller has followed the contract.
 *
 * @param ... The indices of the arguments that are required to be non-null (1-based).
 * If no indices are provided, all pointer arguments are marked non-null.
 */
#define JP_ATTR_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#else
#define JP_ATTR_NONNULL(...)
#pragma message("cc: attribute not supported (nonnull)")
#endif

#if HAS_ATTRIBUTE(access)
/**
 * @brief Informs the compiler that a pointer argument is only used for reading.
 *
 * This attribute enables stronger static analysis by the compiler's -Wstringop-overflow and analyzer passes.
 * It guarantees that the function will not modify the memory pointed to by 'ptr_idx'.
 *
 * It is a more powerful version of 'const' for the compiler,
 * as it defines the specific access intent at the architectural level.
 *
 * @param ptr_idx The 1-based index of the pointer argument.
 */
#define JP_ATTR_READ_ONLY(ptr_idx)              __attribute__((access(read_only, ptr_idx)))
/**
 * @brief Informs the compiler that a pointer is read-only and defines its bound.
 *
 * Tells the optimizer and static analyzer that the function reads 'size_idx' elements from the pointer at 'ptr_idx'.
 * This allows the compiler to detect out-of-bounds reads at compile-time.
 *
 * @param ptr_idx  The 1-based index of the pointer argument.
 * @param size_idx The 1-based index of the argument representing the buffer size.
 */
#define JP_ATTR_READ_ONLY_N(ptr_idx, size_idx)  __attribute__((access(read_only, ptr_idx, size_idx)))
/**
 * @brief Informs the compiler that a pointer argument is used for writing.
 *
 * This attribute hints that the function will modify the memory pointed to by 'ptr_idx'.
 * It helps the compiler's static analyzer detect if uninitialized memory is being passed to a function that
 * expects to write to it, or if a constant pointer is incorrectly passed.
 *
 * Useful for functions that return results via pointer arguments.
 *
 * @param ptr_idx The 1-based index of the pointer argument.
 */
#define JP_ATTR_WRITE_ONLY(ptr_idx)             __attribute__((access(write_only, ptr_idx)))
/**
 * @brief Informs the compiler that a pointer is write-only and defines its bound.
 *
 * This attribute allows the compiler to verify that the buffer pointed to by 'ptr_idx' is large enough
 * to handle 'size_idx' amount of data. It is essential for preventing buffer overflows during output operations.
 *
 * @param ptr_idx  The 1-based index of the pointer argument.
 * @param size_idx The 1-based index of the argument representing the buffer size.
 */
#define JP_ATTR_WRITE_ONLY_N(ptr_idx, size_idx) __attribute__((access(write_only, ptr_idx, size_idx)))
#else
#define JP_ATTR_READ_ONLY(ptr_idx)
#define JP_ATTR_READ_ONLY_N(ptr_idx, size_idx)
#define JP_ATTR_WRITE_ONLY(ptr_idx)
#define JP_ATTR_WRITE_ONLY_N(ptr_idx, size_idx)
#pragma message("cc: attribute not supported (access)")
#endif

#if HAS_ATTRIBUTE(fallthrough)
/**
 * @brief Indicates that a switch case fall-through is intentional.
 *
 * Prevents the compiler from issuing a warning when a 'case' block does not end with a 'break' or 'return'.
 * This clarifies intent to both the compiler's static analyzer and other developers.
 */
#define JP_ATTR_FALLTHROUGH __attribute__((fallthrough))
#else
#define JP_ATTR_FALLTHROUGH
#pragma message("cc: attribute not supported (fallthrough)")
#endif

#if HAS_ATTRIBUTE(nonstring)
/**
 * @brief Marks a character array or pointer as not necessarily null-terminated.
 *
 * This attribute informs the compiler (GCC) that the associated 'char' buffer
 * is used as a fixed-size storage for character data rather than a standard C-style string.
 *
 * Key Benefits:
 * 1. Suppresses warnings like -Wstringop-truncation when copying data into
 * the buffer without adding a null terminator.
 * 2. Triggers a warning if this buffer is passed to functions expecting a
 * null-terminated string (e.g., strlen, printf %s, puts).
 *
 * Useful for fixed-width protocol headers, tags, or database fields.
 */
#define JP_ATTR_BUFFER __attribute__((nonstring))
#else
#define JP_ATTR_BUFFER
#pragma message("cc: attribute not supported (nonstring)")
#endif

#if HAS_ATTRIBUTE(hot)
/**
 * @brief Marks a function as a "hot" execution path.
 *
 * This attribute informs the compiler that the function is very frequently called.
 * The optimizer will:
 * 1. Place the function's machine code in a special "hot" section of the binary to improve instruction cache (i-cache)
 * locality.
 * 2. More aggressively attempt to inline functions called within this function.
 * 3. Prioritize optimizations that favor execution speed over binary size for this specific function.
 *
 * Useful for main processing loops, escape functions, or core parsing logic.
 */
#define JP_ATTR_HOT __attribute__((hot))
#else
#define JP_ATTR_HOT
#pragma message("cc: attribute not supported (hot)")
#endif

#if HAS_ATTRIBUTE(leaf)
/**
 * @brief Specifies that the function is a "leaf" function in the call graph.
 *
 * A leaf function does not call any other functions from the current compilation unit
 * that might call back into the caller (directly or indirectly).
 *
 * This allows the compiler to:
 * 1. Assume that no global variables or memory pointed to by arguments will be
 * modified by "hidden" side effects during the call.
 * 2. Keep local variables in registers across the function call, as it's guaranteed no re-entrancy will occur.
 *
 * @warning Do not use this if the function calls any external library functions or pointers-to-functions
 * that could potentially modify the state of the current module.
 */
#define JP_ATTR_LEAF __attribute__((leaf))
#else
#define JP_ATTR_LEAF
#pragma message("cc: attribute not supported (leaf)")
#endif

#if HAS_ATTRIBUTE(returns_nonnull)
/**
 * @brief Compiler hint guaranteeing that the function never returns a NULL pointer.
 *
 * This macro abstracts the `__attribute__((returns_nonnull))` GCC/Clang attribute.
 * It provides the following benefits:
 * 1. Static Analysis Optimization: Informs tools (Clang-Tidy, Coverity) that
 * NULL-checks on the returned pointer are redundant.
 * 2. Code Generation: Allows the compiler to omit branches that handle NULL
 * return values, resulting in smaller and faster binaries.
 * 3. Undefined Behavior Protection: Functions marked with this attribute that
 * actually return NULL may trigger a trap or undefined behavior, ensuring
 * strict adherence to the "allocate-or-die" policy.
 */
#define JP_ATTR_RET_NONNULL __attribute__((returns_nonnull))
#else
#define JP_ATTR_RET_NONNULL
#pragma message("cc: attribute not supported (returns_nonnull)")
#endif

#if HAS_ATTRIBUTE(alloc_size)
/**
 * @brief Compiler hint for memory allocation size validation.
 *
 * This macro abstracts the `__attribute__((alloc_size(n)))` GCC/Clang attribute.
 * It informs the compiler which argument(s) specify the size of the allocated
 * memory block. This enables:
 * 1. Enhanced static analysis (detecting potential buffer overflows).
 * 2. Optimizations for `__builtin_object_size`.
 * 3. Improved warnings for out-of-bounds access at compile time.
 *
 * @param ... One or two index positions (1-based) of the function arguments
 * representing the allocation size (e.g., size for malloc, nmemb and size for calloc).
 */
#define JP_ATTR_ALLOCS(...) __attribute__((alloc_size(__VA_ARGS__)))
#else
#define JP_ATTR_ALLOCS(...)
#pragma message("cc: attribute not supported (alloc_size)")
#endif

#if !defined(NDEBUG) && HAS_ATTRIBUTE(weak)
/**
 * @brief Marks a function as "weak" to allow mocking in test environments.
 *
 * This macro enables the 'weak' compiler attribute only when NOT in a Release
 * build (i.e., NDEBUG is not defined) and if the compiler supports it.
 *
 * - In Debug/Test: The function is marked as weak, meaning the linker will
 * prioritize any other "strong" definition of the same function (e.g., a Mock).
 * - In Release (NDEBUG): The macro expands to nothing. This ensures that
 * duplicate function definitions trigger a "multiple definition" linker error,
 * preventing accidental test code leakage into production.
 */
#define JP_ATTR_WEAK __attribute__((weak))
#else
#define JP_ATTR_WEAK
#pragma message("cc: NDEBUG not defined or attribute not supported (weak)")
#endif

/**
 * @brief Executes a statement and returns its error code if it is non-zero.
 *
 * This macro captures the result of 'stm' into a temporary variable to avoid double evaluation.
 * If the result is not 0 (standard for errors in POSIX/C), it immediately returns that value from the current function.
 *
 * @note Uses __typeof__ (GCC/Clang extension) for type safety and to support
 * various return types (int, long, etc.) without explicit casting.
 *
 * @param stm The expression or function call to evaluate.
 */
#define JP_VERIFY(stm)                 \
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
#define JP_LOG_INFO(fmt, ...) fprintf(stdout, fmt "\n", ##__VA_ARGS__)

/**
 * @brief Conditional logging macro for runtime messages.
 *
 * Unlike JP_LOG_INFO, this macro is suppressible via the runtime 'quiet' configuration.
 * It appends a newline to every message for consistent formatting.
 * It uses the GNU '##' extension to safely handle cases where no variadic
 * arguments are provided, preventing trailing comma compilation errors.
 *
 * @param fmt Format string (printf-style).
 * @param ... Variadic arguments for the format string.
 */
#define JP_LOG_MSG(fmt, ...)                          \
    ({                                                \
        if (!JP_CONF_SILENT_GET()) {                  \
            fprintf(stdout, fmt "\n", ##__VA_ARGS__); \
        }                                             \
        (void) 0;                                     \
    })

#ifndef NDEBUG
/**
 * @brief Conditionally prints debug information to stdout.
 *
 * - When NDEBUG is not defined, this macro behaves like fprintf, adding a "[DEBUG]" prefix and a newline.
 * - When NDEBUG is defined (Release mode), it evaluates to a no-op, ensuring zero runtime overhead and removing debug
 * strings from the binary.
 *
 * @param fmt Format string (printf-style).
 * @param ... Variadic arguments for the format string.
 */
#define JP_LOG_DEBUG(fmt, ...) fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define JP_LOG_DEBUG(fmt, ...) ((void) 0)
#endif

#undef HAS_BUILTIN
#undef HAS_ATTRIBUTE

#endif  // JPIPE_JP_COMMON_H
