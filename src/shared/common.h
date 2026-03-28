/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define _cleanup_(f) __attribute__((cleanup(f)))
#define _printf_(a, b) __attribute__((__format__(printf, a, b)))
#define _unused_ __attribute__((__unused__))

#define new(t, n) ((t*) reallocarray(NULL, n, sizeof(t)))

#define mfree(memory)                           \
        ({                                      \
                free(memory);                   \
                (typeof(memory)) NULL;          \
        })

static inline void freep(void *p) {
        *(void**)p = mfree(*(void**) p);
}

#define _cleanup_free_ _cleanup_(freep)

#define TAKE_GENERIC(var, type, nullvalue)                       \
        ({                                                       \
                type *_pvar_ = &(var);                           \
                type _var_ = *_pvar_;                            \
                type _nullvalue_ = nullvalue;                    \
                *_pvar_ = _nullvalue_;                           \
                _var_;                                           \
        })
#define TAKE_PTR(ptr) TAKE_GENERIC(ptr, typeof(ptr), NULL)

#define POINTER_MAX ((void*) UINTPTR_MAX)

char* _strjoin(const char *x, ...);
#define strjoin(x, ...) _strjoin(x, __VA_ARGS__, POINTER_MAX);

int log_error_errno(int r, const char *msg, ...) _printf_(2, 3);

#define log_debug_errno(...) log_error_errno(__VA_ARGS__)
