/* path.c - path handling routines
 *
 *   Copyright (C) 2010-2011  Henrik Hautakoski <henrik.hautakoski@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   The design goal here is to not use fixed size buffers because
 *   path's can be extremely long (slightly overexaggerated but extreme cases are extreme).
 *   so we use funky heap memory based algorithms instead :)
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "strbuf.h"
#include "path.h"
#include "log.h"

static char path_null = '\0';

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

int path_isparent(const char *path, const char *parent) {

    if (*path++ != '/' || *parent++ != '/')
        return 0;

    while(*path) {
        if (*parent == '\0')
            return *path == '/' || *(path-1) == '/';
        if (*path++ != *parent++)
            break;
    }
    return 0;
}

const char* real_path(const char *path) {

    struct stat st;
    unsigned numresolve = 5;

    static strbuf_t cwd = STRBUF_INIT, buffers[2] = { STRBUF_INIT };
    strbuf_t *resolve = buffers, *tmpbuf = buffers + 1, file = STRBUF_INIT;

    strbuf_setlen(resolve, 0);
    strbuf_append_str(resolve, path);

    while(numresolve--) {

        char *last = NULL;

        if (!is_dir(resolve->buf)) {
            strbuf_setlen(&file, 0);
            last = strrchr(resolve->buf, '/');
            if (last) {
                strbuf_append_str(&file, last + 1);
                strbuf_setlen(resolve, last - resolve->buf);
            } else {
                last = resolve->buf;
                strbuf_append_str(&file, last);
                strbuf_setlen(resolve, 0);
            }
        }

        if (resolve->len) {
            if (!cwd.len && strbuf_getcwd(&cwd) < 0) {
                logerrno(LOG_CRIT, "getcwd", errno);
                goto err;
            }

            if (chdir(resolve->buf)) {
                logerrno(LOG_WARN, "chdir", errno);
                goto err;
            }
        }
        if (strbuf_getcwd(resolve) < 0) {
            logerrno(LOG_CRIT, "getcwd", errno);
            goto err;
        }

        if (last) {
            strbuf_term(resolve, '/');
            strbuf_append(resolve, file.buf, file.len);
            strbuf_setlen(&file, 0);
        }

        if (!lstat(resolve->buf, &st) && S_ISLNK(st.st_mode)) {
            strbuf_t *tmpnext;
            int len = strbuf_readlink(resolve, tmpbuf->buf);
            if (len < 0) {
                logerrno(LOG_CRIT, "readlink", errno);
                goto out;
            }
            tmpnext = resolve;
            resolve = tmpbuf;
            tmpbuf = tmpnext;
        } else {
            break;
        }
    }

out:
    if (cwd.len && chdir(cwd.buf))
        logerrno(LOG_CRIT, "chdir", errno);

    strbuf_free(&file);
    return resolve->buf;
err:
    strbuf_free(&file);
    return NULL;
}

const char* dirname_s(const char *path, int slash) {

    static strbuf_t sb = STRBUF_INIT;

    if (sb.buf != path) {
        strbuf_setlen(&sb, 0);
        strbuf_append_str(&sb, path);
    }
    strbuf_squeeze(&sb, '/');

    if (sb.len > 1) {
        if (sb.buf[sb.len-1] == '/')
            strbuf_reduce(&sb, 1);

        if (!strbuf_rchop(&sb, '/'))
            strbuf_reduce(&sb, sb.len-1);
    }

    if (sb.buf[0] != '/')
        sb.buf[0] = '.';
    if (slash)
        strbuf_term(&sb, '/');
    return sb.buf;
}

const char* basename_s(const char *path) {

    static strbuf_t sb = STRBUF_INIT;

    strbuf_setlen(&sb, 0);
    strbuf_append_str(&sb, path);
    strbuf_squeeze(&sb, '/');

    if (!sb.len) {
        strbuf_append_ch(&sb, '.');
    } else if (sb.len > 1) {
        char *ptr;

        if (sb.buf[sb.len-1] == '/')
            strbuf_reduce(&sb, 1);

        ptr = strrchr(sb.buf, '/');
        if (ptr++) {
            size_t len = sb.len - (sb.buf - ptr);
            memcpy(sb.buf, ptr, len);
            strbuf_setlen(&sb, len);
        }
    }
    return sb.buf;
}

const char *mkpath(const char *fmt, ...) {

    static strbuf_t sb = STRBUF_INIT;
    va_list va;
    int len;

    if (strbuf_avail(&sb)) {
        strbuf_reduce(&sb, sb.len);
    } else {
        strbuf_expand(&sb, 1);
    }

    va_start(va, fmt);
    len = strbuf_append_va(&sb, fmt, va);
    va_end(va);

    if (len > 0) {
        strbuf_expand(&sb, len);
        va_start(va, fmt);
        strbuf_append_va(&sb, fmt, va);
        va_end(va);
    }

    return sb.buf;
}

static char* expand_home() {

    char *home = getenv("HOME");

    if (!home)
        home = &path_null;
    return home;
}

char* path_normalize(const char *base, const char *name, unsigned dir) {

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
