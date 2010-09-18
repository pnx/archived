/* common/xalloc.h - stricter memory allocation.
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __COMMON_XALLOC_H
#define __COMMON_XALLOC_H

#include <stddef.h>

void* xmalloc(size_t);

void* xmallocz(size_t);

void* xrealloc(void *, size_t);

char* xstrdup(const char *);

void xfree(void *);

#endif /* __COMMON_XALLOC_H */
