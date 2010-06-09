/* common/path.c - path handling routines
 * 
 *   Copyright (C) 2010  Henrik Hautakoski <henrik.hautakoski@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   The design goal here is to not use fixed size buffers because 
 *   path's can be extremely long (slightly overexaggerated but extreme cases are extreme).
 *   so we use funky heap memory based algorithms instead :)
 */

#include <string.h>

#include "debug.h"
#include "path.h"

static char defpath[2];

/*
 * allocates and initilizes a path
 */
static char* alloc_path(size_t s) {
	
	char *ptr;
		
	if (s < 1)
		return NULL;

    ptr = malloc(s+1);
    
	if (ptr == NULL)
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
 * NOTE: buffer should be atleast of size pathlen(path)+1
 */
static char* cpy_path(char *buf, const char *path) {
	
	if (*path == 0)
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

static char* split_path(char *path) {

    char *last = path+strlen(path)-1, *slash = NULL;

    while(*last == '/' && (last--) > path);

    while(last > path) {

        if (*last == '/') {
            slash = last;
        } else if (slash != NULL) {
            break;
        }

        last--;
    }

    if (slash == NULL)
        return path;

    return slash;
}


int is_abspath(const char *path) {
	
	if (*path != '/')
		return 0;
	
	return strstr(path, "/./")  == NULL && 
		   strstr(path, "/../") == NULL;
}

size_t pathlen(const char *path) {
	
	size_t size = 0;
	
	while(*path) {
		
		size++;
		
		if (*path == '/')
			path = path_clear(path);
        else
            path++;
	}

	return size;
}

char* fmt_path(const char *base, const char *name, unsigned char dir) {

	char *ptr, *ret;
	size_t size;
	
	if (base == NULL || !is_abspath(base) || has_delim(name))
		return NULL;
	
	size = pathlen(base);
	
	if (name != NULL) {
		size += strlen(name);
		if (dir)
            size++;
	}

    if (*(base+size) != '/')
        size++;

    ptr = alloc_path(size);
    
	if (ptr == NULL)
		return NULL;
	
	ret = ptr;
	
	ptr = cpy_path(ptr, base);

	if (name != NULL) {
		memcpy(ptr, name, strlen(name));
		if (dir)
            *(ptr+strlen(name)) = '/';
	}
	
	return ret;
}

char* basename(char *path) {

    char *pos = path;

    if (path == NULL || *path == '\0') {
        defpath[0] = '.';
        defpath[1] = '\0';
        return defpath;
    }

    while(*path != '\0') {
        if (*path == '/') {
            if (*(path+1) == '\0') {
                if (pos >= path)
                    break;
                *(path--) = '\0';
                continue;
            }
            if (*(path+1) != '/')
                pos = path+1;
        }
        path++;
    }

    return pos;
}

char* dirname(char *path) {

    char *last, *slash = NULL;
	size_t len;

    if (path == NULL || *path == '\0') {
        defpath[0] = '.';
        defpath[1] = '\0';
        return defpath;
    }

	len = split_path(path) - path;

    if (len <= 1) {
        if (*path != '/') 
            path[0] = '.';
        path[1] = '\0';
    } else {
        path[len] = '\0';
    }

	return path;
}
