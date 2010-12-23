/* str-list.c
 *
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <string.h>

#include "xalloc.h"
#include "str-list.h"

static void resize(struct str_list *list) {

    if (!list->nr) {
        xfree(list->items);
        str_list_init(list);
        return;
    }
    list->items = xrealloc(list->items, sizeof(list->items) * list->nr);
}

static int get_index(struct str_list *list, const char *str, int *match) {

    int min = -1, max = list->nr;

    while(min + 1 < max) {
        int mid = min + (max - min)/2;
        int cmp = strcmp(list->items[mid], str);
        if (cmp < 0) {
            min = mid;
        } else if (cmp > 0) {
            max = mid;
        } else {
            *match = 1;
            return mid;
        }
    }

    *match = 0;
    return max;
}

void str_list_init(struct str_list *list) {

    list->items = NULL;
    list->nr = 0;
}

struct str_list* str_list_create(void) {

    struct str_list *list = xmalloc(sizeof(struct str_list));
    list->items = NULL;
    list->nr = 0;
    return list;
}

int str_list_destroy(struct str_list *list) {

    if (str_list_isempty(list)) {
        xfree(list);
        return 1;
    }
    return 0;
}

void str_list_clear(struct str_list *list) {

    str_list_clear_fn(list, xfree);
}

void str_list_clear_fn(struct str_list *list, void (*fn)(void *)) {

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

int str_list_insert(struct str_list *list, const char *str) {

    int match, index = get_index(list, str, &match);

    if (match)
        return -1;

    list->items = xrealloc(list->items, sizeof(list->items) * (list->nr + 1));
    if (index < list->nr) {
        memmove(list->items + index + 1, list->items + index, 
            sizeof(list->items) * (list->nr - index));
    }
    
    list->items[index] = (char *) str;
    list->nr++;
    
    return index;
}

char* str_list_remove(struct str_list *list, const char *str) {

    char *item = NULL;

    if (list && str) {
        int match, index = get_index(list, str, &match);
        if (match && index < list->nr) {
            item = list->items[index];
            if (index < --list->nr) {
                memmove(list->items + index, list->items + index + 1, 
                    sizeof(list->items) * (list->nr - index));
            }
            resize(list);
        }
    }
    return item;
}

char* str_list_reduce(struct str_list *list) {

    char *item = NULL;

    if (list && list->nr) {
        item = list->items[--list->nr];
        resize(list);
    }
    return item;
}

int str_list_indexof(struct str_list *list, const char *str) {

    int match, index = get_index(list, str, &match);
    return match ? index : -1;
}

char* str_list_lookup(struct str_list *list, const char *str) {

    int match, index = get_index(list, str, &match);

    if (!match)
        return NULL;
    return list->items[index];
}

int str_list_has(struct str_list *list, const char *str) {

    int rc;
    get_index(list, str, &rc);
    return rc;
}

char** str_list_export(struct str_list *list) {

    int i;
    char **out = xmalloc(sizeof(char*) * (list->nr + 1));
    
    for(i=0; i < list->nr; i++)
        out[i] = list->items[i];
    out[list->nr] = NULL;

    return out;
}


