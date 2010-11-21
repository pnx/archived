/* path.c - path handling routines
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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "debug.h"
#include "strbuf.h"
#include "path.h"

static char path_null = '\0';

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

int is_abspath(const char *path) {

	if (path == NULL || *path != '/')
		return 0;

    for(; *path; path++) {

        if (*path == '/' && *(path+1) == '.') {
            path += 2;
            if (*path == '.')
                path++;
            if (*path == '/' || *path == '\0')
                return 0;
        }
    }

	return 1;
}

int is_file(const char *path) {

    struct stat st;

    if (path && stat(path, &st) >= 0)
        return S_ISREG(st.st_mode);
    return 0;
}

int is_dir(const char *path) {

    struct stat st;
    
    if (path && stat(path, &st) >= 0)
        return S_ISDIR(st.st_mode);
    return 0;
}

static char* expand_home() {

    char *home = getenv("HOME");

    if (!home)
        home = &path_null;
    return home;
}

char* path_normalize(const char *base, const char *name, unsigned char dir) {

    strbuf_t sb = STRBUF_INIT;

    if (base == NULL || has_delim(name)) 
		return NULL;

    if (*base == '~') {
        base++;
        strbuf_append_str(&sb, expand_home());
        strbuf_term(&sb, '/');
    }

    strbuf_append_str(&sb, base);
    strbuf_term(&sb, '/');

    if (!is_abspath(sb.buf))
        goto cleanup;

    if (name) {
        strbuf_append_str(&sb, name);
    
        if (dir)
            strbuf_append_ch(&sb, '/');
    }

    strbuf_squeeze(&sb, '/');

    return strbuf_release(&sb);
cleanup:
    strbuf_free(&sb);
    return NULL;
}
