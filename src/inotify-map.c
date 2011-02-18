/* inotify-map.c
 * 
 *  (C) Copyright 2011 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include "xalloc.h"
#include "rbtree.h"
#include "list.h"
#include "path.h"
#include "inotify-watch.h"
#include "inotify-map.h"

static void wd_free(void *ptr);
static int wd_cmp(const void *a, const void *b);
static int path_cmp(const void *a, const void *b);

static struct watch watch_root;

/* wd -> list of struct watch* */
static rbtree tree_wd_paths = RBTREE_INIT(wd_cmp);

/* path -> struct watch* */
static rbtree tree_path_wd = RBTREE_INIT(path_cmp);

static int wd_cmp(const void *a, const void *b) {

    struct watch *n1 = ((struct list*)a)->items[0];
    struct watch *n2 = ((struct list*)b)->items[0];

    return n1->wd - n2->wd;
}

static int path_cmp(const void *a, const void *b) {

    return strcmp(((struct watch*)a)->path, ((struct watch*)b)->path);
}

static void wd_free(void *list) {

    list_destroy(list);
}

static struct watch* new_node(int wd, const char *path) {

    struct watch s, *p;

    s.path = path;

    /* find parent */
    for(;;) {
        s.path = dirname_s(s.path, 1);
        p = rbtree_search(&tree_path_wd, &s);
        if (p)
            break;
        if (!strcmp(s.path, "/") ||
            !strcmp(s.path, "./")) {
            p = &watch_root;
            break;
        }
    }
    return inotify_watch_add(p, inotify_watch_new(wd, path));
}

static struct list* wd_lookup(int wd) {

    struct watch n, *np = &n;
    struct list s = { (void**)&np, 1 };
    n.wd = wd;
    return rbtree_search(&tree_wd_paths, &s);
}

static void map_path(struct watch *w) {

    rbtree_insert(&tree_path_wd, w);
}

static void map_wd(struct watch *w) {

    struct list s = { (void**)&w, 1 };
    struct list *l = rbtree_search(&tree_wd_paths, &s);

    if (l) {
        list_insert(l, w);
    } else {
        l = list_create();
        list_insert(l, w);
        rbtree_insert(&tree_wd_paths, l);
    }
}

void inotify_map(int wd, const char *path) {

    struct watch s, *n;

    s.path = path;
    n = rbtree_search(&tree_path_wd, &s);

    if (!n) {
        n = new_node(wd, path);
        map_path(n);
        map_wd(n);
    }
}

static void unmap_path(struct watch *watch) {

    rbtree_delete(&tree_path_wd, watch);
}

static void unmap_wd(struct watch *watch) {

    int index;
    struct list s = { (void**)&watch, 1 }, *list;

	list = rbtree_search(&tree_wd_paths, &s);
    if (!list)
        return;

    index = list_indexof(list, watch);
    if (index >= 0) {
        if (list_size(list) == 1) {
            list = rbtree_delete(&tree_wd_paths, list);
            list_destroy(list);
        } else {
            list_remove(list, index);
        }
    }
}

static void unmap(struct watch *w) {

    unmap_wd(w);
    unmap_path(w);
}

int inotify_unmap_wd(int wd) {

    struct list *l = wd_lookup(wd);
    if (l) {
        int i;

        rbtree_delete(&tree_wd_paths, l);
        for(i=0; i < l->nr; i++) {
            struct watch *w = l->items[i];
            unmap_path(w);
            inotify_watch_destroy(w, unmap);
        }
        list_destroy(l);
        return 1;
    }
    return 0;
}

int inotify_unmap_path(const char *path) {

    struct watch *w, s;

    s.path = path;
    w = rbtree_search(&tree_path_wd, &s);
    if (w) {
        unmap_wd(w);
        unmap_path(w);
        inotify_watch_destroy(w, unmap);
        return 1;
    }
    return 0;
}

void inotify_unmap_all() {

    rbtree_free(&tree_path_wd, NULL);
    rbtree_free(&tree_wd_paths, wd_free);
    inotify_watch_destroy((struct watch*)watch_root.tree.child, NULL);
    watch_root.tree.child = NULL;
}

int inotify_map_get_wd(const char *path) {

    struct watch s, *n;

    s.path = path;
    n = rbtree_search(&tree_path_wd, &s);
    if (n)
        return n->wd;
    return 0;
}

char** inotify_map_get_path(int wd) {

    char **out = NULL;
    struct list *list;

    list = wd_lookup(wd);
    if (list && list->nr) {
        int i;
        
        out = xmalloc(list->nr + 1);
        for(i=0; i < list->nr; i++) {
            struct watch *w = list->items[i];
            out[i] = (char *)w->path;
        }
        out[list->nr] = NULL;
    }
    return out;
}

char** inotify_map_lookup_by_path(const char *path) {

    int wd = inotify_map_get_wd(path);
    if (wd)
        return inotify_map_get_path(wd);
    return NULL;
}

int inotify_map_isempty() {

    return rbtree_is_empty(&tree_wd_paths)
        && rbtree_is_empty(&tree_path_wd)
        && watch_root.tree.child == NULL;
}
