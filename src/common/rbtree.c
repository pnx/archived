/* common/rbtree.c - red black tree implementation
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

#include "xalloc.h"
#include "debug.h"
#include "rbtree.h"

#define is_red(n) ((n) != NULL && (n)->color == RB_RED)
#define swap(n,d,q) ((n)->child[(n)->child[d] == (q)])

static rbnode* node_alloc(uint key, void *ptr, size_t len) {
	
	rbnode *n = xmalloc(sizeof(rbnode));
	
	n->key      = key;
	n->data     = ptr;
    n->len      = len;
	n->color    = RB_RED;
	n->child[0] = NULL;
	n->child[1] = NULL;
	
	return n;
}

/*
 * Recursivly deallocate a tree.
 */
static void node_dealloc(rbnode *n, void (*action)(rbnode *)) {
	
	if (!n)
		return;

	if (action) {
		action(n);
	} else if (n->data) {
		xfree(n->data);
        n->data = NULL;
    }
	
	node_dealloc(n->child[0], action);
	node_dealloc(n->child[1], action);
	
	xfree(n);
}

/*
 * Recursivly walks a tree, applying action function on every node
 */
static void rbwalk(rbnode *n, void (*action)(rbnode *)) {
	
	if (n == NULL)
		return;
	
	action(n);
	
	rbwalk(n->child[0], action);
	rbwalk(n->child[1], action);
}

static inline int datacmp(void *d1, void *d2, size_t l) {

    if (d1 == NULL || d2 == NULL)
        return d1 == d2;
        
    return !memcmp(d1, d2, l);
}

/*
 * Compares every node's data member with cmpdata along the
 * path. comparison is done at memory level and returns the first node that match.
 */
static rbnode* rbcmp(rbnode *n, void *cmpdata, size_t len) {
	
	rbnode *r;
	
	if (!n)
		return NULL;

    dprint("CMP %s - %s\n", (char*)n->data, (char*)cmpdata);

	if (datacmp(n->data, cmpdata, len))
		return n;
	
	r = rbcmp(n->child[0], cmpdata, len);
	
	if (r == NULL)
		r = rbcmp(n->child[1], cmpdata, len);
	
	return r;
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
rbnode* rbtree_search(rbtree *tree, uint key) {
	
	rbnode *n;
	
	if (tree == NULL || tree->root == NULL)
		return NULL;
	
	n = tree->root;
	
	while(n) {
		
		dprint("SEARCH: check %u\n", n->key);
		
		if (n->key == key)
			break;
		
		n = n->child[n->key < key];
	}
	
	return n;
}

rbnode* rbtree_cmp_search(rbtree *tree, void *cmpdata, size_t len) {
	
	if (tree == NULL)
		return NULL;
				
	return rbcmp(tree->root, cmpdata, len);
}

void rbtree_walk(rbtree *tree, void (*action)(rbnode *)) {
	
	if (tree == NULL)
		return;
			
	rbwalk(tree->root, action);
}

void rbtree_free(rbtree *tree, void (*action)(rbnode *)) {
		
	if (tree == NULL)
		return;
	
	node_dealloc(tree->root, action);
	tree->root = NULL;
}

/*
 * duplicate keys result in the tree remains unchanged
 * this can cause memory leaks as data fields can (should) 
 * be heap allocated, and the client expects us to keep track of it.
 *
 * for general purposes, we should notify client about it so 
 * then they can choose what to do
 *
 * the function now returns -1 in that situation // H Hautakoski
 */
int rbtree_insert(rbtree *tree, uint key, void *data, size_t len) {
	
	rbnode head = {0};
	
	/* grandparent and parent */
	rbnode *g, *t;
	
	/* iterator and parent */
	rbnode *p, *q;
	
	unsigned char dir = 0, dir2, last, inserted = 0;
	
	if (tree->root == NULL) {
		tree->root = node_alloc(key, data, len);
		if (tree->root == NULL)
			return 0;
        goto done;
	}
		
	t = &head;
	g = p = NULL;
	q = t->child[1] = tree->root;

    /* somewhere in here, there should be dragons */
		
	for(;;) {
		if (q == NULL) {
			p->child[dir] = q = node_alloc(key, data, len);
			if (q == NULL)
				return 0;
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
			
		if (q->key == key)
			break;
			
		last = dir;
		dir = q->key < key;
			
		if (g)
			t = g;
		g = p, p = q;
		q = q->child[dir];
	}
		
	tree->root = head.child[1];
    
done:
	/* root should be black */
	tree->root->color = RB_BLACK;
	
    if (!inserted)
        return -1;
    
	return 1;
}

void* rbtree_delete(rbtree *tree, uint key) {
	
	rbnode head = {0};
	
	/* helpers*/
	rbnode *q, *p, *g, *s;
	
	/* found item */
	rbnode *f = NULL;
	
	/* pointer to the data member of the node we delete,
       returned so it can be free'd */
	void *ret = NULL;
	
	unsigned char dir = 1, dir2, last;
	
	if (rbtree_is_empty(tree))
		return NULL;
	
	q = &head;
	g = p = NULL;
	q->child[1] = tree->root;
	
	/* more dragons (killed some of them though) */
	
	while(q->child[dir]) {
		last = dir;
		
		g = p, p = q;
		q = q->child[dir];
		dir = q->key < key;
		
		if (q->key == key)
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
	
	tree->root = head.child[1];
	if (tree->root)
		tree->root->color = RB_BLACK;
	
	/* remove if found */
	if (f) {
        ret = f->data;
        if (f != q) {
            f->key  = q->key;
            f->data = xmemdup(q->data, q->len);
            f->len  = q->len;
        }
        swap(p, 1, q) = swap(q, 0, NULL);
        xfree(q);
	}
	
	return ret;
}
