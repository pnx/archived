/* fscrawl.h - Filesystem traversal
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __FSCRAWL_H
#define __FSCRAWL_H

typedef struct {
    char *name;
    char *base;
    unsigned char dir;
} fs_entry;

typedef struct __fscrawl *fscrawl_t;

fscrawl_t fsc_open(const char *path);

void fsc_close(fscrawl_t f);

fs_entry* fsc_cpy(fs_entry *ent);

fs_entry* fsc_read(fscrawl_t f);

#endif /* __FSCRAWL_H */
