/* strbuf.c
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include "compat/string.h"
#include "util.h"
#include "xalloc.h"
#include "strbuf.h"

#define CHNK_SIZE 128

char strbuf_null = '\0';

void strbuf_init(strbuf_t *s) {

    s->buf = &strbuf_null;
    s->alloc_size = s->len = 0;
}

size_t strbuf_avail(strbuf_t *s) {

    return s->alloc_size ? s->alloc_size - (s->len + 1) : 0;
}

void strbuf_expand(strbuf_t *s, size_t len) {

    if (s->len + len + 1 < s->alloc_size)
        return;
    if (!s->alloc_size)
        s->buf = NULL;

    do
        s->alloc_size += CHNK_SIZE;
    while(s->len + len + 1 > s->alloc_size);

    s->buf = xrealloc(s->buf, s->alloc_size);
}

void strbuf_reduce(strbuf_t *s, size_t len) {

    if (len > s->len)
        len = s->len;

    strbuf_setlen(s, s->len - len);
}

void strbuf_setlen(strbuf_t *s, size_t len) {

    if (!s->alloc_size)
        return;

    if (len >= s->alloc_size)
        len = s->alloc_size - 1;
    s->len = len;
    s->buf[s->len] = '\0';
}

char* strbuf_release(strbuf_t *s) {

    char *ret;

    if (!s->alloc_size)
        ret = xmallocz(1);
    else if (s->len + 1 != s->alloc_size)
        ret = xrealloc(s->buf, s->len + 1);
    else
        ret = s->buf;

    strbuf_init(s);

    return ret;
}

void strbuf_free(strbuf_t *s) {

    if (!s->alloc_size)
        return;

    xfree(s->buf);
    strbuf_init(s);
}

void strbuf_free_list(strbuf_t **s) {

    strbuf_t **list = s;

    while(*s) {
        if ((*s)->alloc_size)
            xfree((*s)->buf);
        xfree(*s++);
    }
    xfree(list);
}

void strbuf_attach(strbuf_t *s, void *buf, size_t len, size_t alloc_size) {

    strbuf_free(s);
    s->buf = buf;
    s->len = len;
    s->alloc_size = alloc_size;
    strbuf_expand(s, 0);
    s->buf[s->len] = '\0';
}

void strbuf_append(strbuf_t *s, const void *ptr, size_t len) {

    strbuf_expand(s, len);
    memcpy(s->buf + s->len, ptr, len);
    strbuf_setlen(s, s->len + len);
}

void strbuf_appendf(strbuf_t *s, const char *fmt, ...) {

    va_list va;
    int len;

    if (!strbuf_avail(s))
        strbuf_expand(s, CHNK_SIZE-1);

    va_start(va, fmt);
    len = strbuf_append_va(s, fmt, va);
    va_end(va);

    if (len > 0) {
        strbuf_expand(s, len);
        va_start(va, fmt);
        len = strbuf_append_va(s, fmt, va);
        va_end(va);
    }
}

int strbuf_append_va(strbuf_t *s, const char *fmt, va_list va) {

    int len = vsnprintf(s->buf + s->len, s->alloc_size - s->len, fmt, va);
    if (len < 0)
        die_errno("vsnprintf");
    if (len > strbuf_avail(s))
        return len;
    s->len += len;
    s->buf[len] = '\0';
    return 0;
}

void strbuf_append_str(strbuf_t *s, const char *str) {

    strbuf_append(s, str, strlen(str));
}

void strbuf_append_ch(strbuf_t *s, char ch) {

    strbuf_expand(s, 1);
    s->buf[s->len++] = ch;
    s->buf[s->len] = '\0';
}

void strbuf_append_repeat(strbuf_t *s, char ch, size_t len) {

    strbuf_expand(s, len);
    memset(s->buf + s->len, ch, len);
    strbuf_setlen(s, s->len + len);
}

char* strbuf_rchop(strbuf_t *s, char ch) {

    char *n = memrchr(s->buf, ch, s->len);

    if (n)
        strbuf_setlen(s, n - s->buf);
    return n;
}

void strbuf_term(strbuf_t *s, char ch) {

    if (s->buf[s->len-1] != ch)
        strbuf_append_ch(s, ch);
}

void strbuf_trim(strbuf_t *s) {

    strbuf_rtrim(s);
    strbuf_ltrim(s);
}

void strbuf_rtrim(strbuf_t *s) {

    while(s->len && isspace(s->buf[s->len-1]))
         s->len--;
    s->buf[s->len] = '\0';
}

void strbuf_ltrim(strbuf_t *s) {

    size_t i, of = 0;

    for(; of < s->len && isspace(s->buf[of]); of++);

    if (of < 1)
        return;

    s->len -= of;
    for(i=0; i <= s->len; i++)
        s->buf[i] = s->buf[i + of];
}

void strbuf_rev(strbuf_t *s) {

    char tmp, *st = s->buf, *en = s->buf + s->len - 1;

    while(st < en) {
        tmp = *st;
        *st++ = *en;
        *en-- = tmp;
    }
}

void strbuf_squeeze(strbuf_t *s, char ch) {

    size_t p, np, of;

    for(p=s->len; p; p--) {

        if (s->buf[p-1] != ch)
            continue;

        np = p;

        for(of = 0; np-1 && s->buf[np-2] == ch; np--)
            of++;

        if (of) {
            p = np;
            for(s->len -= of; np <= s->len; np++)
                s->buf[np] = s->buf[np + of];
        }
    }
}

strbuf_t** strbuf_explode(const strbuf_t *s, char sep) {

    int i = 0, count = 2;
    char *p;
    strbuf_t *tmp, **list;

    list = xmallocz(sizeof(strbuf_t *) * count);

    p = s->buf;
    while(p < s->buf + s->len) {
        char *d = memchr(p, sep, s->len - (p - s->buf));
        if (!d)
            d = s->buf + s->len;
        if (i + 1 >= count)
            list = xrealloc(list, sizeof(strbuf_t *) * (count += 4));
        tmp = xmalloc(sizeof(strbuf_t));
        strbuf_init(tmp);
        strbuf_append(tmp, p, d - p);
        list[i] = tmp;
        list[++i] = NULL;
        p = ++d;
    }
    return list;
}
