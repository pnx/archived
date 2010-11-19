/* fscrawl.c - Filesystem traversal
 * 
 *  Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>

#include "strbuf.h"
#include "path.h"
#include "debug.h"
#include "fscrawl.h"

#define MAX_DEPTH 0x20

#define isrefdir(x) (!strcmp((x)->d_name, ".") || !strcmp((x)->d_name, ".."))

struct __fscrawl {
	strbuf_t path;
	DIR  *dirs[MAX_DEPTH];
	unsigned char depth;
    fs_entry ent;
};

static int mvup(struct __fscrawl *f) {
	
	if (closedir(f->dirs[f->depth]) == -1) {
		perror("closedir");
        errno = 0;
		return -1;
	}
	
	if (f->depth == 0)
		return 0;
        
    dprint("fscrawl: tree depth is: %i \n", f->depth);
    
	f->depth--;

    if (f->path.buf[f->path.len-1] == '/')
        strbuf_reduce(&f->path, 1);
    strbuf_rchop(&f->path, '/');
    strbuf_term(&f->path, '/');

	return 1;
}

static int mvdown(struct __fscrawl *f, const char *dir) {

	DIR  *ds = NULL;
	
	if (f->depth >= MAX_DEPTH-1)
		return 0;

    strbuf_term(&f->path, '/');
    strbuf_append_str(&f->path, dir);
    strbuf_term(&f->path, '/');

    ds = opendir(f->path.buf);

	if (!ds) {
        if (errno != EACCES)
            perror("opendir");
        errno = 0;
        
		strbuf_reduce(&f->path, 1);
        strbuf_rchop(&f->path, '/');
        strbuf_term(&f->path, '/');
        return 0;
	}
	
	f->dirs[++f->depth] = ds;
	
	return 1;
}

fscrawl_t fsc_open(const char *path) {
	
	struct __fscrawl *f = malloc(sizeof(struct __fscrawl));
	
	if (f == NULL)
		return NULL;

    strbuf_init(&f->path);

    char *npath = path_normalize(path, NULL, 1);

    if (!npath)
        return NULL;

    /* set the normalized path as the string buffer */
    strbuf_attach(&f->path, npath, strlen(npath), strlen(npath));

	f->depth = 0;

	f->dirs[f->depth] = opendir(f->path.buf);
	
	if (!f->dirs[f->depth]) {
        
        perror("fsc_open");
        errno = 0;
        
        strbuf_free(&f->path);
		free(f);
		return NULL;
	}
	
	f->ent.name = NULL;
	f->ent.base = NULL;
	f->ent.dir  = 0;
	
	return f;
}

void fsc_close(fscrawl_t f) {
	
	int i;
	
	for(i=0; i < f->depth; i++)
		closedir(f->dirs[i]);
	
    strbuf_free(&f->path);
	free(f);
}

fs_entry* fsc_cpy(fs_entry *ent) {
	
	void *ptr = malloc(sizeof(fs_entry));
	
	if (ptr == NULL)
		return NULL;
	
	memcpy(ptr, ent, sizeof(fs_entry));
	
	return ptr;
}

fs_entry* fsc_read(fscrawl_t f) {

	struct dirent *ent = NULL;

	if (!f)
		return NULL;

    if (f->ent.dir)
        mvdown(f, f->ent.name);

	for(;;) {
		
		ent = readdir(f->dirs[f->depth]);

        if (ent) {
            if (isrefdir(ent))
                continue;
            break;
        }

        if (errno) {
            perror("fsc_read");
            errno = 0;
            continue;
        }
			
        if (mvup(f))
            continue;
			
        fsc_close(f);
        return NULL;
	}

    f->ent.base = f->path.buf;
	f->ent.name = &ent->d_name[0];
	f->ent.dir  = ent->d_type == 4;

    return &f->ent;
}
