/* rbtree.h
 * 
 *   Copyright (C) 2010  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __RBTREE_H
#define __RBTREE_H

#define RB_RED   0
#define RB_BLACK 1

#include <stddef.h>

/* node definition */
typedef struct _rbn {
    const void   *key;
	struct _rbn  *child[2];
	unsigned char color;
} rbnode;

typedef struct {
	rbnode *root;
    /* user defined operations */
    void (*delete_fn)(void *);
    void (*update_fn)(void *, void *);
    int  (*cmp_fn)(const void *, const void *);
} rbtree;

#define RBTREE_INIT(delete, update, cmp) { NULL, delete, update, cmp}

int rbtree_is_empty(rbtree *tree);

void* rbtree_search(rbtree *tree, const void *key);

void rbtree_walk(rbtree *tree, void (*action)(const void *));

void rbtree_free(rbtree *tree);

int rbtree_insert(rbtree *tree, const void *key);

int rbtree_delete(rbtree *tree, const void *key);

#endif /* __RBTREE_H */
