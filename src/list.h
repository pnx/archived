/* list.h
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __LIST_H
#define __LIST_H

#include <stddef.h>

struct list {
    void   **items;
    unsigned nr;
};

typedef void (clear_fn_t)(void*);
typedef int (cmp_fn_t)(const void *, const void *b);

struct list* list_create(void);

struct list* list_copy(struct list *list);

int list_destroy(struct list *list);

void list_clear(struct list *list);

void list_clear_fn(struct list *list, clear_fn_t *fn);

int list_insert(struct list *list, const void *item);

void* list_remove(struct list *list, unsigned index);

void* list_reduce(struct list *list);

int list_indexof(struct list *list, const void *item);

void* list_lookup(struct list *list, const void *item, cmp_fn_t *fn);

#define list_has(l, i, f) list_lookup(l, i, f) != NULL

#define list_size(l) ((l) ? (l)->nr : 0)

#define list_isempty(l) (!(l) || (l)->nr == 0)

#endif /* __LIST_H */
