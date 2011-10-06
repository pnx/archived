/* memrchr.c
 *
 *   memrchr implementation for systems without it
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#include "string.h"

void* memrchr(const void *s, int c, size_t n) {

    for(s += n++; n; n--)
        if (*((char*)s--) == c)
            return (void*) s + 1;
    return NULL;
}
