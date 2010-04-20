
/*
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

/*
 * the design goal here is to not use buffers because 
 * path's can be extremely long (slightly overexaggerated but extreme cases are extreme).
 * so we use funky heap memory based algorithms instead :)
 */

#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "path.h"

/*
 * allocates and initilizes a path
 */
static char* alloc_path(size_t s) {
	
	char *ptr;
		
	if (s < 1)
		return NULL;
		
	if ((ptr = malloc(s+1)) == NULL)
		return NULL;
		
	*ptr = '/';
	memset(ptr+1, 0, s);
		
	return ptr;
}

static const char* path_clear(const char *path) {
	
	while(*path == '/')
		path++;
	
	return path;
}

static inline int has_delim(const char *str) {
	
	if (str == NULL)
		return 0;
	
	return strchr(str, '/') != NULL;
}

/*
 * copy a clean path to buf
 * NOTE: buffer should be atleast of size pathlen(path)
 */
static char* cpy_path(char *buf, const char *path) {
	
	if(!*path)
		return buf;
	
	while(*path) {
		
		*(buf++) = *path;
		
		if (*path == '/')
			path = path_clear(path);
		else
			path++;
	}

	if (*(buf-1) != '/')
		*(buf++) = '/';
	
	dassert(*buf == 0);
	
	return buf;
}

int abspath(const char *path) {
	
	if (*path != '/')
		return 0;
	
	return strstr(path, "/./")  == NULL && 
		   strstr(path, "/../") == NULL;
}

size_t pathlen(const char *path) {
	
	size_t size = 0;
	
	while(*path != 0) {
		
		size++;
		
		if (*path == '/')
			path = path_clear(path);
		else if (*(++path) == 0)
			size++;
	}
	
	return size;
}

char* fmt_path(const char *base, const char *name, unsigned char dir) {

	char *ptr, *ret;
	size_t size;
	
	if (base == NULL || !abspath(base) || has_delim(name))
		return NULL;
	
	size = pathlen(base);
	
	if (name != NULL) {
		size += strlen(name);
		if (dir) size++;
	}
	
	if((ptr = alloc_path(size)) == NULL)
		return NULL;
	
	ret = ptr;
	
	ptr = cpy_path(ptr, base);
	
	dassert((ptr-ret) == pathlen(base));
	
	if (name != NULL) {
		memcpy(ptr, name, strlen(name));
		if (dir) *(ptr+strlen(name)) = '/';
	}
	
	return ret;
}

char* split_path(const char *path) {
	return NULL;
}

const char* basename(const char *path) {
	
	const char *last = path+strlen(path);

	if (*last == '/')
		last--;
	
	while(path != last) {
		if(*last == '/')
			break;
		last--;
	}
	
	return last;
}
