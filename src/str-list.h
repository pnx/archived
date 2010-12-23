/* list.h
 *
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef __STR_LIST_H
#define __STR_LIST_H

#include <stddef.h>

struct str_list {
    char **items;
    size_t nr;
};

void str_list_init(struct str_list *list);

struct str_list* str_list_create(void);

int str_list_destroy(struct str_list *list);

void str_list_clear(struct str_list *list);

void str_list_clear_fn(struct str_list *list, void (*fn)(void *));

int str_list_insert(struct str_list *list, const char *str);

char* str_list_remove(struct str_list *list, const char *str);

char* str_list_reduce(struct str_list *list);

int str_list_indexof(struct str_list *list, const char *str);

char* str_list_lookup(struct str_list *list, const char *str);

int str_list_has(struct str_list *list, const char *str);

#define str_list_foreach(i, l) \
    for(i = (l)->items; i < (l)->items + (l)->nr; ++i)

#define str_list_size(l) ((l) ? (l)->nr : 0)

#define str_list_isempty(l) (!(l) || (l)->nr == 0)

#endif /* __STR_LIST_H */
