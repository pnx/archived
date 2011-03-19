/* fscrawl.c - Filesystem traversal
 *
 *  Copyright (C) 2010-2011   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fts.h>

#include "log.h"
#include "path.h"
#include "fscrawl.h"

#define FTSENT_VALID_TYPE(x)        \
    ((x)->fts_info == FTS_D   ||    \
     (x)->fts_info == FTS_DC  ||    \
     (x)->fts_info == FTS_F   ||    \
     (x)->fts_info == FTS_SL  ||    \
     (x)->fts_info == FTS_SLNONE)

struct __fscrawl {
    FTS *fts;
    fs_entry ent;
};

static FTS* _fts_open(char *path, int opts) {

    char *npath[2] = { path, NULL };

    return fts_open(npath, opts, NULL);
}

static fs_entry* ftsentcpy(fs_entry *dest, FTSENT *src) {

    int len = src->fts_pathlen - src->fts_namelen;

    if (len > 0) {
        char *newbase = realloc(dest->base, len + 1);

        if (newbase) {
            memcpy(newbase, src->fts_path, len);
            newbase[len] = '\0';

            dest->base = newbase;
            dest->name = src->fts_name;
            dest->dir = src->fts_info == FTS_D;
            return dest;
        }
    }

    return NULL;
}

fscrawl_t fsc_open(const char *path) {

    struct __fscrawl *f;
    char *npath;

    f = malloc(sizeof(struct __fscrawl));
    if (!f)
        return NULL;

    npath = path_normalize(path, NULL, 1);
    if (!npath)
        goto free;

    f->fts = _fts_open(npath, FTS_LOGICAL);
    free(npath);

    if (!f->fts) {
        logerrno(LOG_WARN, "ftc_open", errno);
        goto free;
    }

    f->ent.base = NULL;
    f->ent.name = NULL;
    f->ent.dir = 0;
    return f;
free:
    if (f)
        free(f);
    return NULL;
}

void fsc_close(fscrawl_t f) {

    if (f) {
        if (f->ent.base)
            free(f->ent.base);
        if (fts_close(f->fts) < 0)
            logerrno(LOG_WARN, "ftc_close", errno);
        free(f);
    }
}

fs_entry* fsc_cpy(fs_entry *ent) {

    void *ptr = malloc(sizeof(fs_entry));

    if (ptr == NULL)
        return NULL;

    memcpy(ptr, ent, sizeof(fs_entry));

    return ptr;
}

fs_entry* fsc_read(fscrawl_t f) {

    FTSENT *ent;

    if (!f)
        return NULL;

    for(;;) {
        ent = fts_read(f->fts);

        if (ent == NULL) {
            if (errno) {
                logerrno(LOG_DEBUG, "fsc_read", errno);
                continue;
            }
            break;
        }

        if (ent->fts_namelen > 0 && FTSENT_VALID_TYPE(ent))
            return ftsentcpy(&f->ent, ent);

        if (ent->fts_info == FTS_ERR)
            logerrno(LOG_DEBUG, "fsc_read", ent->fts_errno);
    }

    return NULL;
}
