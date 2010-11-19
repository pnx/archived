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
	unsigned      key;
    void         *data;
	struct _rbn  *child[2];
	unsigned char color;
} rbnode;

typedef struct {
	rbnode *root;
    /* user defined operations */
    void (*delete_fn)(void *);
    void (*update_fn)(void *, void *);
    /* note: char* is used here to make dereference easier inside the function */
    int  (*cmp_fn)(const char *, const char *);
} rbtree;

#define RBTREE_INIT(delete, update, cmp) { NULL, delete, update, cmp}

int rbtree_is_empty(rbtree *tree);

rbnode* rbtree_search(rbtree *tree, unsigned key);

rbnode* rbtree_cmp_search(rbtree *tree, const void *data);

void rbtree_walk(rbtree *tree, void (*action)(rbnode *));

void rbtree_free(rbtree *tree);

int rbtree_insert(rbtree *tree, unsigned key, void *data);

int rbtree_delete(rbtree *tree, unsigned key);

#endif /* __RBTREE_H */
