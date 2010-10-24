/* xalloc.c
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "xalloc.h"

#define CHECK(expr) \
    if (!(expr)) \
        goto bail

#define CHECK_SET_ERRNO(expr, no) \
    if (!(expr)) { \
        errno = no; \
        goto bail; \
    }

#ifdef __DEBUG__
# define CHECK_INPUT(s) CHECK_SET_ERRNO(s, EINVAL)
#else
# define CHECK_INPUT(s)
#endif

void* xmalloc(size_t size) {

    CHECK_INPUT(size);

    void *ptr = malloc(size);
    CHECK(ptr);
    return ptr;
bail:
    die_errno("xmalloc");
}

void* xmallocz(size_t size) {

    CHECK_INPUT(size);

    void *ptr = malloc(size);
    CHECK(ptr);
    memset(ptr, 0, size);
    return ptr;
bail:
    die_errno("xmallocz");
}

void* xrealloc(void *ptr, size_t size) {

    CHECK_INPUT(size);
    
    ptr = realloc(ptr, size);
    CHECK(ptr);
    return ptr;
bail:
    die_errno("xrealloc");
}

char* xstrdup(const char *s) {

    CHECK_INPUT(s);

    size_t len = strlen(s) + 1;
    char *dest = xmalloc(len);

    CHECK(dest);
    memcpy(dest, s, len);
    return dest;
bail:
    die_errno("xstrdup");
}

void* xmemdup(const void *src, size_t size) {

    CHECK_INPUT(src);

    void *dest = malloc(size);
    CHECK(dest);
    memcpy(dest, src, size);
    return dest;
bail:
    die_errno("xmemdup");
}

void xfree(void *ptr) {

    CHECK_INPUT(ptr);
    free(ptr);
    return;
bail:
    die_errno("xfree");
}
