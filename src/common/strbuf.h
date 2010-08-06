/* common/strbuf.h
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __COMMON_STRBUF_H
#define __COMMON_STRBUF_H

#include <string.h>

#define STRBUF_INIT { 0, 0, NULL }

typedef struct {
    size_t alloc_size;
    size_t len;
    char  *buf;
} strbuf_t;

void strbuf_init(strbuf_t *s);

void strbuf_append(strbuf_t *s, char *str, size_t len);

void strbuf_reduce(strbuf_t *s, size_t len);

void strbuf_trim(strbuf_t *s);

void strbuf_rtrim(strbuf_t *s);

void strbuf_ltrim(strbuf_t *s);

void strbuf_rev(strbuf_t *s);

char* strbuf_release(strbuf_t *s);

void strbuf_free(strbuf_t *s);

#endif /* __COMMON_STRBUF_H */
