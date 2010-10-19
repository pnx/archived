/* rbtree.h
 * 
 *   Copyright (C) 2010  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __COMMON_RBTREE_H
#define __COMMON_RBTREE_H

#define RB_RED   0
#define RB_BLACK 1

#include <stddef.h>

typedef unsigned char color_t;
typedef unsigned int uint;

/* node definition */
typedef struct _rbn {
	uint 		 key;
	void		*data;
    size_t       len;
	struct _rbn *child[2];
	color_t      color;
} rbnode;

typedef struct {
	rbnode *root;
} rbtree;

int rbtree_is_empty(rbtree *tree);

rbnode* rbtree_search(rbtree *tree, uint key);

rbnode* rbtree_cmp_search(rbtree *tree, void *cmpdata, size_t len);

void rbtree_walk(rbtree *tree, void (*action)(rbnode *));

void rbtree_free(rbtree *tree, void (*action)(rbnode *));

int rbtree_insert(rbtree *tree, uint key, void *data, size_t len);

void* rbtree_delete(rbtree *tree, uint key);

#endif /* _COMMON_RBTREE_H */
