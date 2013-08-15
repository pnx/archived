/* tree.h - N-tree implementation
 *
 *   Copyright (C) 2011   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __TREE_H
#define __TREE_H

#include <stddef.h>

struct tree {
    struct tree *next;
    struct tree *parent;
    struct tree *child;
};

typedef void (*tree_traverse_fn)(struct tree *tree, void *data);

#define tree_is_root(n)      \
    ((n) &&                  \
     (n)->parent == NULL &&  \
     (n)->next == NULL)

#define tree_is_leaf(n) \
    ((n) && (n)->child == NULL)

#define TREE_INIT { NULL }

struct tree* tree_new();

struct tree* tree_link(struct tree *parent, struct tree *tree);

void tree_unlink(struct tree *tree);

void tree_move(struct tree *dest, struct tree *src);

void tree_detach(struct tree *tree);

void tree_traverse(struct tree *tree, tree_traverse_fn fn, void *data);

size_t tree_parent_count(struct tree *tree);

#endif /* __TREE_H */
