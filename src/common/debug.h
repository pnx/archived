/* common/debug.h - debugging macros and definitions
 *
 *   Copyright (C) 2010  Henrik Hautakoski <henrik.hautakoski@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __COMMON_DEBUG_H

#define __COMMON_DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#define __perror(...) fprintf(stderr, __VA_ARGS__)

#define die(...) \
	__perror("DIE: " __VA_ARGS__); \
	exit(1)

#ifdef __DEBUG__

#define __str(x) #x

#define dassert(expr) \
	((expr) ? \
		(void) 0 : \
		__perror("ASSERT: \"" #expr "\" at %s:%i\n", __FILE__, __LINE__))

#define dprint(...) \
	__dprint(__FILE__, __LINE__, __VA_ARGS__)

#define __dprint(file, line, ...) \
    __perror("# " file ":" __str(line) " -> " __VA_ARGS__)

#else
#define dassert(expr)
#define dprint(...)
#endif

#endif /* __COMMON_DEBUG_H */
