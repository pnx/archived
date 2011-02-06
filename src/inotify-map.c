/* inotify-map.c
 * 
 *  (C) Copyright 2011 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <string.h>
#include "xalloc.h"
#include "rbtree.h"
#include "inotify-map.h"

/*
 * IMPORTANT: always have the member used for comparison as the first member
 * to make it possible to cast a struct pointer to the first member type.
 */
struct map_wd {
    int wd;
    struct str_list paths;
};

struct map_path {
    const char *path;
    int wd;
};

static void wd_free(void *ptr);
static int wd_cmp(const void *a, const void *b);
static int path_cmp(const void *a, const void *b);

static rbtree tree_wd_paths = RBTREE_INIT(wd_cmp);
static rbtree tree_path_wd = RBTREE_INIT(path_cmp);

static int wd_cmp(const void *a, const void *b) {

    return *((int*)a) - *((int*)b);
}

static int path_cmp(const void *a, const void *b) {

    return strcmp(*((char**)a), *((char**)b));
}

static void wd_free(void *ptr) {

    struct map_wd *m = (struct map_wd*)ptr;

    str_list_clear(&m->paths);
    xfree(m);
}

static void map_path_wd(const char *path, int wd) {

    struct map_path *p = rbtree_search(&tree_path_wd, &path);
    if (!p) {
        p = xmalloc(sizeof(struct map_path));
        p->path = path;
        rbtree_insert(&tree_path_wd, p);
    }
    p->wd = wd;
}

static int unmap_path_wd(int wd, const char *path) {

    return rbtree_delete(&tree_path_wd, &path) != NULL;
}

static void unmap_wd_path(int wd, const char *path) {

    struct map_wd *m = rbtree_search(&tree_wd_paths, &wd);

    if (m) {
        char *p = str_list_remove(&m->paths, path);
        if (p) {
            xfree(p);
            if (str_list_isempty(&m->paths))
                rbtree_delete(&tree_wd_paths, m);
        }
    }
}

/*
 * unmaps all paths from the 'struct map_wd' and from the search tree 'tree_path_wd'.
 */
static void unmap_all_paths(struct map_wd *map) {

    char *path;

    while(path = str_list_reduce(&map->paths)) {
        rbtree_delete(&tree_path_wd, &path);
        xfree(path);
    }
}

void inotify_map(int wd, const char *path) {

    struct map_wd *m = rbtree_search(&tree_wd_paths, &wd);
    if (!m) {
        m = xmalloc(sizeof(struct map_wd));
        m->wd = wd;
        str_list_init(&m->paths);
        rbtree_insert(&tree_wd_paths, m);
    }

    if (!str_list_has(&m->paths, path)) {
        int index = str_list_insert(&m->paths, xstrdup(path));
        map_path_wd(m->paths.items[index], wd);
    }
}

int inotify_unmap_wd(int wd) {
    
    struct map_wd *m = rbtree_search(&tree_wd_paths, &wd);
    if (m) {
        unmap_all_paths(m);
        return rbtree_delete(&tree_wd_paths, &wd) != NULL;
    }
    return 0;
}

int inotify_unmap_path(const char *path) {

    struct map_path *m = rbtree_search(&tree_path_wd, &path);
    
    if (m) {
        int wd = m->wd;
        unmap_path_wd(wd, path);
        unmap_wd_path(wd, path);
        return 1;
    }
    return 0;
}

void inotify_unmap_all() {

    rbtree_free(&tree_path_wd, xfree);
    rbtree_free(&tree_wd_paths, wd_free);
}

int inotify_map_get_wd(const char *path) {

    struct map_path *p = rbtree_search(&tree_path_wd, &path);
    if (p)
        return p->wd;
    return 0;
}

char** inotify_map_lookup(int wd) {
    
    struct map_wd *m = rbtree_search(&tree_wd_paths, &wd);
    return m ? str_list_export(&m->paths) : NULL;
}

char** inotify_map_lookup_by_path(const char *path) {

    struct map_path *p = rbtree_search(&tree_path_wd, &path);
    return p ? inotify_map_lookup(p->wd) : NULL;
}

int inotify_map_isempty() {

    return rbtree_is_empty(&tree_wd_paths)
        && rbtree_is_empty(&tree_path_wd);
}
