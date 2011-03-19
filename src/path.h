/* path.h - path handling routines
 *
 *   Copyright (C) 2010-2011  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __PATH_H
#define __PATH_H

#include <stddef.h>
#include <libgen.h>

int is_abspath(const char *path);

int is_file(const char *path);

int is_dir(const char *path);

int path_isparent(const char *path, const char *parent);

const char* dirname_s(const char *path, int slash);

const char* mkpath(const char *fmt, ...);

char* path_normalize(const char *base, const char *name, unsigned dir);

#endif /* __PATH_H */
