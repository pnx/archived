/*
 * Modified version of Julienne Walker's implementation
 * http://www.eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
 * 
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
 * 
 * Do not touch anything in this file. it's perfect ;)
 */
#include <malloc.h>

#include "debug.h"
#include "rbtree.h"

#define is_red(n) (n != NULL && n->color == RB_RED)
#define swap(n,d,q) n->child[n->child[d] == q]

#ifdef __DEBUG__
int rb_assert(rbnode *node) {
	
	int rh, lh;
	rbnode *ln;
	rbnode *rn;
	
	if (node == NULL) {
		return 1;
	}
	
	ln = node->child[0];
	rn = node->child[1];
		
	if (is_red(node)) {
		if (is_red(ln) || is_red(rn)) {
			die("Double red");
			return 0;
		}
	}
		
	lh = rb_assert(ln);
	rh = rb_assert(rn);
	
	if ( (ln != NULL && ln->key >= node->key) &&
		 (rn != NULL && rn->key <= node->key) ) {
		die("BST violation");
		return 0;
	}
	
	if (rh != 0 && lh != 0) {
		
		if (rh != lh) {
			die("Black height violation");
			return 0;
		}
		
		return (is_red(node)) ? lh : lh+1;
	}
	
	return 0;
}
#endif

static rbnode* node_alloc(uint key, void *ptr) {
	
	rbnode *n = malloc(sizeof(rbnode));
	
	if (n == NULL)
		return NULL;
	
	n->key   = key;
	n->data  = ptr;
	n->color = RB_RED;
	n->child[0] = NULL;
	n->child[1] = NULL;
	
	return n;
}

static void node_dealloc(rbnode *n, void (*a)(rbnode *)) {
	
	if (n == NULL)
		return;
	
	if (a != NULL)
		a(n);
	else
		free(n->data);
	
	node_dealloc(n->child[0], a);
	node_dealloc(n->child[1], a);
	
	free(n);
}

static void _rbwalk(rbnode *n, void (*a)(rbnode *)) {
	
	if (n == NULL)
		return;
	
	a(n);
	
	_rbwalk(n->child[0], a);
	_rbwalk(n->child[1], a);
}

static rbnode* _rbcmp(rbnode *n, void *d, size_t l) {
	
	rbnode* r;
	
	if (n == NULL)
		return NULL;
	
	if (memcmp(n->data, d, l) == 0)
		return n;
	
	r = _rbcmp(n->child[0], d, l);
	
	if (r == NULL)
		r = _rbcmp(n->child[1], d, l);
	
	return r;
}

static rbnode* rotate_single(rbnode *root, unsigned char dir) {
	
	rbnode *save = root->child[!dir];
	
	root->child[!dir] = save->child[dir];
	save->child[dir]  = root;
	
	root->color = RB_RED;
	save->color = RB_BLACK;
	
	return save;
}

static rbnode* rotate_double(rbnode *root, unsigned char dir) {
	root->child[!dir] = rotate_single(root->child[!dir], !dir);
	return rotate_single(root, dir);
}

int rbtree_insert(rbtree *tree, uint key, void *data) {
	
	rbnode head = {0};
	
	/* grandparent and parent */
	rbnode *g, *t;
	
	/* iterator and parent */
	rbnode *p, *q;
	
	unsigned char dir = 0, dir2, last;
	
	/* somewhere in here, there should be dragons */
	
	if (tree->root == NULL) {
		tree->root = node_alloc(key, data);
		if (tree->root == NULL)
			return 0;
	} else {
		
		t = &head;
		g = p = NULL;
		q = t->child[1] = tree->root;
		
		for(;;) {
			if (q == NULL) {
				p->child[dir] = q = node_alloc(key, data);
				if (q == NULL)
					return 0;
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
			
			if (g != NULL)
				t = g;
			g = p, p = q;
			q = q->child[dir];
		}
		
		tree->root = head.child[1];
	}
	
	/* root should be black */
	tree->root->color = RB_BLACK;
	
	return 1;
}

void* rbtree_delete(rbtree *tree, uint key) {
	
	rbnode head = {0};
	
	/* helpers*/
	rbnode *q, *p, *g, *s;
	
	/* found item */
	rbnode *f = NULL;
	
	/* pointer to the data member of the node we delete */
	void *ret = NULL;
	
	unsigned char dir = 1, dir2, last;
	
	if (rbtree_is_empty(tree))
		return NULL;
	
	q = &head;
	g = p = NULL;
	q->child[1] = tree->root;
	
	/* more dragons */
	
	while(q->child[dir] != NULL) {
		last = dir;
		
		g = p, p = q;
		q = q->child[dir];
		dir = q->key < key;
		
		if (q->key == key)
			f = q;
		
		if (!is_red(q) && !is_red(q->child[dir])) {
			if (is_red(q->child[!dir]))
				p = p->child[last] = rotate_single(q, dir);
			else if (!is_red(q->child[!dir])) {
				s = p->child[!last];
				
				if (s != NULL) {
				    /* swap color if both child's are black */
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
		}
	}
	
	tree->root = head.child[1];
	if (tree->root != NULL)
		tree->root->color = RB_BLACK;
	
	/* remove if found */
	if (f != NULL) {
		ret = q->data;
		if (q == tree->root) {
			tree->root = NULL;
		} else {
			if (f != q) {
				f->key  = q->key;
				f->data = q->data;
			}
			swap(p,1,q) = swap(q, 0, NULL);
		}
		free(q);
	}
	
	return ret;
}

rbnode* rbtree_search(rbtree *tree, uint key) {
	
	rbnode *n;
	
	if (tree == NULL || tree->root == NULL)
		return NULL;
	
	n = tree->root;
	
	while(n != NULL) {
		
#ifdef __DEBUG__
		printf("SEARCH: check %u\n", n->key);
#endif
		
		if (n->key == key)
			break;
		
		n = n->child[n->key < key];
	}
	
	return n;
}

rbnode* rbtree_cmp_search(rbtree *tree, void *cmpdata, size_t len) {
	
	if (tree == NULL)
		return NULL;
				
	return _rbcmp(tree->root, cmpdata, len);
}

void rbtree_walk(rbtree *tree, void (*action)(rbnode *)) {
	
	if (tree == NULL)
		return;
			
	_rbwalk(tree->root, action);
}

void rbtree_free(rbtree *tree, void (*action)(rbnode *)) {
		
	if (tree == NULL)
		return;
	
	node_dealloc(tree->root, action);
	tree->root = NULL;
}

inline int rbtree_is_empty(rbtree *tree) {

	return tree == NULL || tree->root == NULL;
}
