/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <stdarg.h>
#include <string.h>

#include "common.h"

int log_error_errno(int r, const char *msg, ...) {
        errno = -abs(r);

        va_list ap;
        va_start(ap, msg);
        vprintf(msg, ap);
        puts("");
        va_end(ap);

        return r;
}

char* _strjoin(const char *x, ...) {
        va_list ap;
        size_t l = 0;
        char *y, *p;

        va_start(ap, x);
        for (const char *t = x; t != POINTER_MAX; t = va_arg(ap, const char*))
                if (t)
                        l += strlen(t);
        va_end(ap);

        y = p = new(char, l + 1);
        if (!y)
                return NULL;

        va_start(ap, x);
        for (const char *t = x; t != POINTER_MAX; t = va_arg(ap, const char*))
                if (t)
                        p = stpcpy(p, t);
        va_end(ap);

        *p = '\0';

        return y;
}
