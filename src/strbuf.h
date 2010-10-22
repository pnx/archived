/* strbuf.h
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

#include <stddef.h>

typedef struct {
    size_t alloc_size;
    size_t len;
    char  *buf;
} strbuf_t;

extern char strbuf_null;

#define STRBUF_INIT { 0, 0, &strbuf_null }

void strbuf_init(strbuf_t *s);

void strbuf_expand(strbuf_t *s, size_t len);

void strbuf_reduce(strbuf_t *s, size_t len);

char* strbuf_release(strbuf_t *s);

void strbuf_free(strbuf_t *s);

void strbuf_attach(strbuf_t *s, void *str, size_t len, size_t alloc_size);

void strbuf_append(strbuf_t *s, const void *ptr, size_t len);

void strbuf_append_str(strbuf_t *s, const char *str);

void strbuf_append_ch(strbuf_t *s, char ch);

void strbuf_rchop(strbuf_t *s, char ch);

void strbuf_term(strbuf_t *s, char ch);

void strbuf_trim(strbuf_t *s);

void strbuf_rtrim(strbuf_t *s);

void strbuf_ltrim(strbuf_t *s);

void strbuf_rev(strbuf_t *s);

void strbuf_squeeze(strbuf_t *s, char ch);

#endif /* __COMMON_STRBUF_H */