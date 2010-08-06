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
#include <malloc.h>
#include <assert.h>
#include "strbuf.h"

#define CHNK_SIZE 128

static void buf_expand(strbuf_t *s, size_t l) {

	if (l <= s->alloc_size)
		return;

	do 
		s->alloc_size += CHNK_SIZE;
	while(l > s->alloc_size);
	
    s->buf = realloc(s->buf, s->alloc_size);
	assert(s->buf != NULL);
}

void strbuf_init(strbuf_t *s) {

	s->buf = NULL;
	s->len = 0;
	s->alloc_size = 0;
}

void strbuf_append(strbuf_t *s, char *str, size_t len) {

	if (str == NULL)
		return;

    buf_expand(s, s->len + len + 1);
	
    memcpy(s->buf + s->len, str, len);
    s->len += len;
    s->buf[s->len] = '\0';
}

void strbuf_reduce(strbuf_t *s, size_t len) {

    if (len > s->len)
        len = s->len;

    s->len -= len;
    s->buf[s->len] = '\0';
}

void strbuf_trim(strbuf_t *s) {

	strbuf_rtrim(s);
	strbuf_ltrim(s);
}

void strbuf_rtrim(strbuf_t *s) {

    for(; s->len > 0 && isspace(s->buf[s->len-1]); s->len--);
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

char* strbuf_release(strbuf_t *s) {

	char *ret;
	
	if (s->len + 1 != s->alloc_size) {
		ret = realloc(s->buf, s->len + 1);
		assert(ret != NULL);
	} else {
		ret = s->buf;
	}
	
	strbuf_init(s);
	
	return ret;
}

void strbuf_free(strbuf_t *s) {

    if (s->alloc_size > 0)
        free(s->buf);
    s->buf = NULL;
}


