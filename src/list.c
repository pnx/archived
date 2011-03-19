/* list.c
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#include <string.h>
#include "xalloc.h"
#include "list.h"

static void resize(struct list *l) {

    if (!l->nr) {
        xfree(l->items);
        l->items = NULL;
        return;
    }
    l->items = xrealloc(l->items, sizeof(l->items) * l->nr);
}

struct list* list_create(void) {

    struct list *list = xmalloc(sizeof(struct list));
    list->items = NULL;
    list->nr = 0;
    return list;
}

struct list* list_copy(struct list *list) {

    if (list && list->nr) {
        struct list *copy = xmalloc(sizeof(struct list));
        copy->items = xmemdup(list->items, sizeof(list->items) * list->nr);
        copy->nr = list->nr;
        return copy;
    }
    return NULL;
}

int list_destroy(struct list *list) {

    list_clear_fn(list, NULL);
    xfree(list);
    return 0;
}

void list_clear(struct list *list) {

    list_clear_fn(list, NULL);
}

void list_clear_fn(struct list *list, clear_fn_t *fn) {

    if (list->items) {
        if (fn) {
            int i;
            for(i=0; i < list->nr; i++)
                fn(list->items[i]);
        }
        xfree(list->items);
    }
    list->items = NULL;
    list->nr = 0;
}

int list_insert(struct list *list, const void *item) {

   if (!list)
             return -1;

    list->items = xrealloc(list->items, sizeof(list->items) * (++list->nr));
    list->items[list->nr - 1] = (void *) item;

    return list->nr;
}

void* list_remove(struct list *list, unsigned index) {

    void *item = NULL;

    if (list && index < list->nr) {
        if (index < --list->nr) {
            memmove(list->items + index, list->items + index + 1,
                sizeof(list->items) * (list->nr - index));
        }
        resize(list);
    }
    return item;
}

void* list_reduce(struct list *list) {

    void *item = NULL;

    if (list && list->nr) {
        item = list->items[--list->nr];
        resize(list);
    }
    return item;
}

int list_indexof(struct list *list, const void *item) {

    int i;

    for(i=0; i < list->nr; i++) {

    if (list->items[i] == item)
        return i;
    }
    return -1;
}

void* list_lookup(struct list *list, const void *item, cmp_fn_t *fn) {

    int i;

    if (fn) {
        for(i=0; i < list->nr; i++) {
            if (fn(list->items[i], item) == 0)
                return list->items[i];
        }
    } else {
        i = list_indexof(list, item);
        if (i >= 0)
            return list->items[i];
    }
    return NULL;
}
