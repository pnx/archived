/* rbtree.c - red black tree implementation
 *
 *   Copyright (C) 2010-2011  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Based on the work of Julienne Walker's rbtree implementation
 *   http://www.eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
 */
#include <stdlib.h>
#ifdef __DEBUG__
#include <assert.h>
#include <stdio.h>
#endif
#include "xalloc.h"
#include "rbtree.h"

#define RB_RED   0
#define RB_BLACK 1

/* node definition */
typedef struct _rbn {
    const void   *key;
    struct _rbn  *child[2];
    unsigned char color;
} rbnode;

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

#ifdef __DEBUG__
#define rb_err(msg) fputs("rbtree error: " msg, stderr)

static int rb_assert_r(rbnode *node, int (*cmp)(const void *, const void *)) {

    int rh, lh;
    rbnode *ln, *rn;

    if (node == NULL)
        return 1;

    ln = node->child[0];
    rn = node->child[1];

    if (is_red(node)) {
        /* double red violation */
        if (is_red(ln) || is_red(rn)) {
            rb_err("Double red violation");
            return 0;
        }
    }

    lh = rb_assert_r(ln, cmp);
    rh = rb_assert_r(rn, cmp);

    if ( (ln && cmp(ln->key, node->key) >= 0) &&
         (rn && cmp(rn->key, node->key) <= 0) ) {
        rb_err("Binary tree violation");
        return 0;
    }

    if (rh != 0 && lh != 0) {

        if (rh != lh) {
            rb_err("Black height violation");
            return 0;
        }

        return is_red(node) ? lh : lh+1;
    }

    return 0;
}
#undef rb_err

static void rb_assert(rbtree *tree) {

    if (!tree || !tree->cmp_fn)
        return;

    assert(rb_assert_r(tree->root, tree->cmp_fn));
}
#endif /* __DEBUG__ */

/*
 * Recursivly walks a tree, applying action function on every node
 */
static void rbwalk(rbnode *n, void (*action)(const void *)) {

    if (!n)
        return;

    rbwalk(n->child[0], action);
    action(n->key);
    rbwalk(n->child[1], action);
}

static rbnode* rotate_single(rbnode *root, int dir) {

    rbnode *save = root->child[!dir];

    root->child[!dir] = save->child[dir];
    save->child[dir]  = root;

    root->color = RB_RED;
    save->color = RB_BLACK;

    return save;
}

static rbnode* rotate_double(rbnode *root, int dir) {
    root->child[!dir] = rotate_single(root->child[!dir], !dir);
    return rotate_single(root, dir);
}

int rbtree_is_empty(rbtree *tree) {

    return tree == NULL || tree->root == NULL;
}

/*
 * Searches a tree by key.
 */
void* rbtree_search(rbtree *tree, const void *key) {

    rbnode *n;

    if (!tree || !tree->cmp_fn)
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

    if (!tree || !action)
        return;

    rbwalk(tree->root, action);

#ifdef __DEBUG__
    rb_assert(tree);
#endif
}

void rbtree_free(rbtree *tree, void (*free_fn)(void *)) {

    if (!tree)
        return;

    node_dealloc(tree->root, free_fn);
    tree->root = NULL;

#ifdef __DEBUG__
    rb_assert(tree);
#endif
}

int rbtree_insert(rbtree *tree, const void *key) {

    rbnode head = {0};

    /* grandparent and parent */
    rbnode *g, *t;

    /* iterator and parent */
    rbnode *p, *q;

    int dir = 0, last = 0;

    if (!tree || !tree->cmp_fn)
        return -1;

    if (!tree->root) {
        tree->root = q = node_alloc(key);
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
        } else if (is_red(q->child[0]) && is_red(q->child[1])) {
            /* color flip case */
            q->color = RB_RED;
            q->child[0]->color = RB_BLACK;
            q->child[1]->color = RB_BLACK;
        }

        /* fix red validation */
        if (is_red(q) && is_red(p)) {
            int dir2 = (t->child[1] == g);
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

    tree->root = head.child[1];

done:
    /* root should be black */
    tree->root->color = RB_BLACK;

#ifdef __DEBUG__
    rb_assert(tree);
#endif

    return q && q->key == key;
}

void* rbtree_delete(rbtree *tree, const void *key) {

    rbnode head = {0};

    /* helpers*/
    rbnode *q, *p, *g, *s;

    /* found item */
    rbnode *f = NULL, *ret = NULL;

    int dir = 1, dir2, last;

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
        ret = (void*)f->key;
        if (f != q)
            f->key = q->key;
        swap(p, 1, q) = swap(q, 0, NULL);
        xfree(q);
    }

    tree->root = head.child[1];
    if (tree->root)
        tree->root->color = RB_BLACK;

#ifdef __DEBUG__
    rb_assert(tree);
#endif

    return ret;
}
