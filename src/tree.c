/* tree.c - N-tree implementation
 *
 *   Copyright (C) 2011   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Generic implementation of a N-tree
 */

#include "xalloc.h"
#include "tree.h"

struct tree* tree_new() {

    return xmallocz(sizeof(struct tree));
}

struct tree* tree_link(struct tree *parent, struct tree *tree) {

    if (parent->child) {
        struct tree *it = parent->child;
        while(it->next)
            it = it->next;
        it->next = tree;
    } else {
        parent->child = tree;
    }
    tree->parent = parent;
    return tree;
}

void tree_unlink(struct tree *tree) {

    if (tree_is_root(tree))
        return;

    /* remove the node from the tree */
    if (tree->parent->child == tree) {
        tree->parent->child = tree->next;
    } else {
        struct tree *prev = tree->parent->child;
        while(prev->next && prev->next != tree)
            prev = prev->next;
        prev->next = tree->next;
    }

    /* move the children to node's old parent */
    if (tree->child) {
        struct tree *it = tree->parent->child;
        if (tree->parent->child) {
            while(it->next)
                it = it->next;
            it->next = tree->child;
        } else {
            tree->parent->child = tree->child;
        }

        /* Update parent */
        for(it = tree->child; it; it = it->next)
            it->parent = tree->parent;
    }

    tree->parent = NULL;
    tree->next = NULL;
    tree->child = NULL;
}

void tree_move(struct tree *dest, struct tree *src) {

    if (src->parent == dest)
        /* node are already in place */
        return;

    /* detach and attach att the new node */
    tree_detach(src);
    tree_link(dest, src);
}

void tree_detach(struct tree *tree) {

    if (tree_is_root(tree))
        return;

    if (tree->parent->child == tree) {
        tree->parent->child = tree->next;
    } else if (tree->parent->child) {
        struct tree *prev = tree->parent->child;
        while(prev->next && prev->next != tree)
            prev = prev->next;
        prev->next = tree->next;
    }
    tree->parent = NULL;
    tree->next = NULL;
}

void tree_traverse(struct tree *tree, tree_traverse_fn fn, void *data) {

    while(tree) {
        struct tree *next = tree->next;
        fn(tree, data);
        if (tree->child)
            tree_traverse(tree->child, fn, data);
        tree = next;
    }
}

size_t tree_parent_count(struct tree *tree) {

    size_t count = 0;
    
    if (tree) {
        for(; tree->parent; count++)
            tree = tree->parent;
    }
    return count;
}
