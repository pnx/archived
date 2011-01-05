/* rbtree.c - red black tree implementation
 * 
 *   Copyright (C) 2010  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Based on the work of Julienne Walker's rbtree implementation
 *   http://www.eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
 */
#include <stdlib.h>
#include <string.h>
#include "xalloc.h"
#include "rbtree.h"

#define is_red(n) ((n) != NULL && (n)->color == RB_RED)
#define swap(n,d,q) ((n)->child[(n)->child[d] == (q)])

static rbnode* node_alloc(const void *key) {
	
	rbnode *n = xmalloc(sizeof(rbnode));
	
	n->key      = key;
	n->color    = RB_RED;
	n->child[0] = NULL;
	n->child[1] = NULL;
	
	return n;
}

/*
 * Recursivly deallocate a tree.
 */
static void node_dealloc(rbnode *n, void (*dealloc)(void *)) {
	
	if (!n)
		return;

    if (dealloc)
        dealloc((void *)n->key);

	node_dealloc(n->child[0], dealloc);
	node_dealloc(n->child[1], dealloc);

	free(n);
}

/*
 * Recursivly walks a tree, applying action function on every node
 */
static void rbwalk(rbnode *n, void (*action)(const void *)) {
	
	if (n == NULL)
		return;
	
	rbwalk(n->child[0], action);
    action(n->key);
	rbwalk(n->child[1], action);
}

static inline rbnode* rotate_single(rbnode *root, unsigned char dir) {
	
	rbnode *save = root->child[!dir];
	
	root->child[!dir] = save->child[dir];
	save->child[dir]  = root;
	
	root->color = RB_RED;
	save->color = RB_BLACK;
	
	return save;
}

static inline rbnode* rotate_double(rbnode *root, unsigned char dir) {
	root->child[!dir] = rotate_single(root->child[!dir], !dir);
	return rotate_single(root, dir);
}

inline int rbtree_is_empty(rbtree *tree) {

	return tree == NULL || tree->root == NULL;
}

/*
 * Searches a tree by key.
 */
void* rbtree_search(rbtree *tree, const void *key) {
	
	rbnode *n;
	
	if (tree == NULL || tree->root == NULL || tree->cmp_fn == NULL)
		return NULL;
	
	n = tree->root;
	
	while(n) {
        int cmp = tree->cmp_fn(n->key, key);
        
		if (cmp == 0)
			return (void *) n->key;

        n = n->child[cmp < 0];
	}
	return NULL;
}

void rbtree_walk(rbtree *tree, void (*action)(const void *)) {
	
	if (tree == NULL || action == NULL)
		return;
			
	rbwalk(tree->root, action);
}

void rbtree_free(rbtree *tree) {
		
	if (tree == NULL)
		return;
	
	node_dealloc(tree->root, tree->delete_fn);
	tree->root = NULL;
}

int rbtree_insert(rbtree *tree, const void *key) {
	
	rbnode head = {0};
	
	/* grandparent and parent */
	rbnode *g, *t;
	
	/* iterator and parent */
	rbnode *p, *q;
	
	unsigned char dir = 0, dir2, last, inserted = 0;

    if (!tree || !tree->cmp_fn)
        return 0;

	if (tree->root == NULL) {
		tree->root = node_alloc(key);
        inserted = 1;
        goto done;
	}
		
	t = &head;
	g = p = NULL;
	q = t->child[1] = tree->root;

    /* somewhere in here, there should be dragons */
		
	for(;;) {
        int cmp;
        
		if (q == NULL) {
			p->child[dir] = q = node_alloc(key);
            inserted = 1;
		} else if (is_red(q->child[0]) && is_red(q->child[1])) {
			/* color flip case */
			q->color = RB_RED;
			q->child[0]->color = RB_BLACK;
			q->child[1]->color = RB_BLACK;
		}
			
		/* fix red validation */
		if (is_red(q) && is_red(p)) {
			dir2 = (t->child[1] == g);
			if (q == p->child[last])
				t->child[dir2] = rotate_single(g, !last);
			else
				t->child[dir2] = rotate_double(g, !last);
		}

        cmp = tree->cmp_fn(q->key, key);
			
		if (cmp == 0)
			break;
			
		last = dir;
        dir = cmp < 0;
			
		if (g)
			t = g;
		g = p, p = q;
		q = q->child[dir];
	}

    if (!inserted) {
        if (tree->delete_fn)
            tree->delete_fn((void*)q->key);
        q->key = key;
    }

	tree->root = head.child[1];

done:
	/* root should be black */
	tree->root->color = RB_BLACK;

    return inserted;
}

int rbtree_delete(rbtree *tree, const void *key) {
	
	rbnode head = {0};
	
	/* helpers*/
	rbnode *q, *p, *g, *s;
	
	/* found item */
	rbnode *f = NULL;
	
	unsigned char dir = 1, dir2, last;
	
	if (rbtree_is_empty(tree) || !tree->cmp_fn)
		return 0;
	
	q = &head;
	g = p = NULL;
	q->child[1] = tree->root;
	
	/* more dragons (killed some of them though) */
	
	while(q->child[dir]) {
        int cmp;
		
		g = p, p = q;
        q = q->child[dir];
        last = dir;
        
        cmp = tree->cmp_fn(q->key, key);
		dir = cmp < 0;
		if (cmp == 0)
			f = q;
		
		if (is_red(q) || is_red(q->child[dir]))
            continue;
            
        if (is_red(q->child[!dir])) {
            p = p->child[last] = rotate_single(q, dir);
        } else {
            s = p->child[!last];
				
            if (s == NULL)
                continue;

            if (!is_red(s->child[!last]) && !is_red(s->child[last])) {
                p->color = RB_BLACK;
                s->color = q->color = RB_RED;
            } else {
                dir2 = (g->child[1] == p);
						
                if (is_red(s->child[last]))
                    g->child[dir2] = rotate_double(p, last);
                else if (is_red(s->child[!last]))
                    g->child[dir2] = rotate_single(p, last);
						
                q->color = g->child[dir2]->color = RB_RED;
                g->child[dir2]->child[0]->color  = RB_BLACK;
                g->child[dir2]->child[1]->color  = RB_BLACK;
            }
		}
	}

	/* remove if found */
	if (f) {
        if (tree->delete_fn)
            tree->delete_fn((void*)f->key);

        if (f != q)
            f->key = q->key;
        swap(p, 1, q) = swap(q, 0, NULL);
        xfree(q);
	}

    tree->root = head.child[1];
	if (tree->root)
		tree->root->color = RB_BLACK;

	return f != NULL;
}
