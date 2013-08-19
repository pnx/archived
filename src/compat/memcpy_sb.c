/* memcpy_sb.c
 *
 *   Copy memory area with size-boundry check.
 *
 *   Copyright (C) 2013   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#include "string.h"

size_t memcpy_sb(void *dest, size_t dsize, const void *src, size_t ssize) {

    if (dsize >= ssize)
        dsize = ssize;

    /* Let memcpy do the work. */
    memcpy(dest, src, dsize);

    return dsize;
}
