/* common/die.c - functionality for killing things in creative ways.
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include "util.h"

void die(const char *err, ...) {

    va_list vl;
    va_start(vl, err);
    vfprintf(stderr, err, vl);
    va_end(vl);
    exit(EXIT_FAILURE);
}

void die_errno(const char *desc) {

    if (errno)
        perror(desc);
    exit(EXIT_FAILURE);
}
