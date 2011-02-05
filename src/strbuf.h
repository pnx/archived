/* strbuf.h
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __STRBUF_H
#define __STRBUF_H

#include <stddef.h>
#include <stdarg.h>

typedef struct {
    size_t alloc_size;
    size_t len;
    char  *buf;
} strbuf_t;

extern char strbuf_null;

#define STRBUF_INIT { 0, 0, &strbuf_null }

void strbuf_init(strbuf_t *s);

size_t strbuf_avail(strbuf_t *s);

void strbuf_expand(strbuf_t *s, size_t len);

void strbuf_reduce(strbuf_t *s, size_t len);

void strbuf_setlen(strbuf_t *s, size_t len);

char* strbuf_release(strbuf_t *s);

void strbuf_free(strbuf_t *s);

void strbuf_free_list(strbuf_t **s);

void strbuf_attach(strbuf_t *s, void *str, size_t len, size_t alloc_size);

void strbuf_append(strbuf_t *s, const void *ptr, size_t len);

void strbuf_appendf(strbuf_t *s, const char *fmt, ...);

int strbuf_append_va(strbuf_t *s, const char *fmt, va_list va);

void strbuf_append_str(strbuf_t *s, const char *str);

void strbuf_append_ch(strbuf_t *s, char ch);

void strbuf_append_repeat(strbuf_t *s, char ch, size_t len);

void strbuf_rchop(strbuf_t *s, char ch);

void strbuf_term(strbuf_t *s, char ch);

void strbuf_trim(strbuf_t *s);

void strbuf_rtrim(strbuf_t *s);

void strbuf_ltrim(strbuf_t *s);

void strbuf_rev(strbuf_t *s);

void strbuf_squeeze(strbuf_t *s, char ch);

strbuf_t** strbuf_explode(const strbuf_t *s, char sep);

#endif /* __STRBUF_H */
