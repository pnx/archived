/*
 * Common Debugging macros and definitions
 * 
 * Copyright (C) 2010  Archived
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _COMMON_DEBUG_H

#define _COMMON_DEBUG_H

#define __perror(...) fprintf(stderr, __VA_ARGS__)

#define die(...) \
	__perror("DIE: " __VA_ARGS__); \
	exit(1)

#ifdef __DEBUG__

#include <stdio.h>
#include <stdlib.h>

#define dassert(expr) \
	((expr) ? \
		(void) 0 : \
		__perror("ASSERT: \"" #expr "\" at %s:%i\n", __FILE__, __LINE__))

#define dprint(...) \
	__perror("DEBUG: " __VA_ARGS__)

#else
#define dassert(expr)
#define dprint(...)
#endif

#endif /* _COMMON_DEBUG_H */
