/* common/strbuf.c
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#include <ctype.h>
#include <string.h>
#include "xalloc.h"
#include "strbuf.h"
#include "debug.h"

#define CHNK_SIZE 128

char strbuf_null = '\0';

void strbuf_init(strbuf_t *s) {

	s->buf = &strbuf_null;
	s->alloc_size = s->len = 0;
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

    s->len -= len;
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
    s->len += len;
    s->buf[s->len] = '\0';
}

void strbuf_append_str(strbuf_t *s, const char *str) {

    strbuf_append(s, str, strlen(str));
}

void strbuf_append_ch(strbuf_t *s, char ch) {

    strbuf_expand(s, 1);
    s->buf[s->len++] = ch;
    s->buf[s->len] = '\0';
}

void strbuf_rchop(strbuf_t *s, char ch) {

    int i;
    
    for(i=s->len-1; i >= 0; i--) {

        if (s->buf[i] == ch) {
            s->buf[i] = '\0';
            s->len = i;
            break;
        }
    }
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

    for(; s->len && isspace(s->buf[s->len-1]); s->len--);
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

    size_t p;

    if (s->len <= 1)
        return;

    for(p=s->len-1; p; p--) {

        if (s->buf[p] != ch)
            continue;

        size_t np = p, of = 0;
        
        for(; np && s->buf[np-1] == ch; np--)
            of++;
        for(s->len -= of; np <= s->len; np++)
            s->buf[np] = s->buf[np + of];
    }
}
