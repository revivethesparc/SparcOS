#pragma once
#include <stdint.h>
#include <stddef.h>

/*
 * @brief  Standard function descriptor — add fields here to extend all
 *         function metadata uniformly.
 */
typedef struct {
    const char* name;
    const char* description;
    const char* version;
    const void* func_ptr;
} func_info_t;

/*
 * @brief  Boilerplate that wraps a function definition with its descriptor
 * @param  n  Function name (C identifier)
 * @param  d  One-line description string
 * @param  v  Semver string
 *
 * The function is `static inline` (no duplicate-linkage issues when the
 * header is included from multiple TUs).  The descriptor lives in the
 * `.func_info` ELF section so it can be discovered at runtime.
 */
#define FUNC(n, d, v)                                                       \
    static inline __attribute__((unused)) void n(void);                     \
    __attribute__((used, section(".func_info")))                             \
    static const func_info_t n##_info = {                                    \
        .name        = #n,                                                   \
        .description = d,                                                    \
        .version     = v,                                                    \
        .func_ptr    = (const void*)n,                                       \
    };                                                                       \
    static inline __attribute__((unused)) void n(void)
