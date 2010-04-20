
/* 
 * Copyright (C) 2010  Archived
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _COMMON_RBTREE_H

#define _COMMON_RBTREE_H

#define RB_RED   0
#define RB_BLACK 1

typedef unsigned char color_t;
typedef unsigned int uint;

/* node definition */
typedef struct _rbn {
	uint 		 key;
	void		*data;
	struct _rbn *child[2];
	color_t      color;
} rbnode;

typedef struct {
	rbnode *root;
} rbtree;

#ifdef RB_DEBUG
int rb_assert(rbnode *node);
#endif

int rbtree_insert(rbtree *tree, uint key, void *data);

void* rbtree_delete(rbtree *tree, uint key);

rbnode* rbtree_search(rbtree *tree, uint key);

rbnode* rbtree_cmp_search(rbtree *tree, void *cmpdata, size_t len);

void rbtree_walk(rbtree *tree, void (*action)(rbnode *));

void rbtree_free(rbtree *tree, void (*action)(rbnode *));

inline int rbtree_is_empty(rbtree *tree);

#endif /* _COMMON_RBTREE_H */
