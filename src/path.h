/* path.h - path handling routines
 * 
 *   Copyright (C) 2010  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __COMMON_PATH_H
#define __COMMON_PATH_H

#include <stddef.h>
#include <libgen.h>

int is_abspath(const char *path);

size_t pathlen(const char *path);

char* path_normalize(const char *base, const char *name, unsigned char dir);

#endif /* __COMMON_PATH_H */