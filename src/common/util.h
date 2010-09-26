/* common/util.h
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __COMMON_UTIL_H
#define __COMMON_UTIL_H

#include <unistd.h>

void die(const char *, ...);

void die_errno(const char *);

#define file_exists(s) (access((s), F_OK) == 0)

#endif /* __COMMOT_UTIL_H */
