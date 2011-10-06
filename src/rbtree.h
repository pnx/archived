/* rbtree.h
 *
 *   Copyright (C) 2010-2011  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __RBTREE_H
#define __RBTREE_H

#include <stddef.h>

typedef struct {
    struct _rbn *root;
    /* user defined operations */
    int  (*cmp_fn)(const void *, const void *);
} rbtree;

#define RBTREE_INIT(cmp) { NULL, cmp }

int rbtree_is_empty(rbtree *tree);

void* rbtree_search(rbtree *tree, const void *key);

void rbtree_walk(rbtree *tree, void (*action)(const void *));

void rbtree_free(rbtree *tree, void (*free_fn)(void *));

int rbtree_insert(rbtree *tree, const void *key);

void* rbtree_delete(rbtree *tree, const void *key);

#endif /* __RBTREE_H */
