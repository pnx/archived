
/*
 * rb_assert should be called after every rbtree_* operation (not just only the one's
 * that actualy performs changes to the tree) to ensure the tree is valid after the
 * function is done.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "unit.h"
#include "../src/rbtree.h"

#define NODES   3000
#define MAX_VAL (3*(NODES/4))

#define is_red(n) ((n) != NULL && (n)->color == RB_RED)

static int vcmp(const void *a, const void *b);
static void vdelete(void *ptr);

/* data */
static rbtree tree = RBTREE_INIT(vdelete, NULL, vcmp);
static int   keyref[NODES];

static int rb_assert(rbnode *node) {
	
	int rh, lh;
	rbnode *ln, *rn;
	
	if (node == NULL) {
		return 1;
	}
	
	ln = node->child[0];
	rn = node->child[1];
		
	if (is_red(node)) {
        /* double red violation */
		if (is_red(ln) || is_red(rn)) {
			die("Double red");
			return 0;
		}
	}
		
	lh = rb_assert(ln);
	rh = rb_assert(rn);

    if ( (ln != NULL && vcmp(ln->key, node->key) >= 0) &&
         (rn != NULL && vcmp(rn->key, node->key) <= 0) ) {
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

static int vcmp(const void *a, const void *b) {

    return *((int*)a) - *((int*)b);
}

static void vdelete(void *ptr) {

    int i, exists = 0;

    for(i=0; i < NODES; i++) {

        if (ptr == &keyref[i]) {
            exists = 1;
            break;
        }
    }
    
    assert(exists);
}

/* Helper, will check keyref to see if 'value' exist in the range */
static int keyref_exists(int low, int high, int value) {

    for(; low <= high; low++)
        if (keyref[low] == value)
            return 1;
    return 0;
}

static void walk_fn(rbnode *n) {

    static int i = 0;

    while(keyref_exists(0, i-1, keyref[i]))
        i++;

    assert(keyref[i] == *((int*)n->key));

    if (++i > NODES)
        i = 0;
}

static void setup(int sorted) {

    int i;

    utest_init_RNG();

    for(i=0; i < NODES; i++)
        keyref[i] = rand() % MAX_VAL;

    if (sorted)
        qsort(keyref, NODES, sizeof(keyref[0]), vcmp);
        
    for(i=0; i < NODES; i++) {
        rbtree_insert(&tree, &keyref[i]);
        rb_assert(tree.root);
    }
}

static void teardown() {

    int i;

    rbtree_free(&tree);
    rb_assert(tree.root);

    for(i=0; i < NODES; i++)
        keyref[i] = -1;
}

void test_rbtree_is_empty() {

    setup(0);

    assert(rbtree_is_empty(&tree) == 0);
    rb_assert(tree.root);

    rbtree_free(&tree);
    rb_assert(tree.root);

    assert(rbtree_is_empty(&tree));
    rb_assert(tree.root);

    teardown();
}

void test_rbtree_delete() {

    int i;

    setup(0);

    for(i=NODES/2; i < 3*(NODES/4); i++) {

        if (keyref_exists(NODES/2, i-1, keyref[i]))
            continue;
        
        assert(rbtree_delete(&tree, &keyref[i]));
        rb_assert(tree.root);
    }

    /* delete a key that does not exist */
    i = MAX_VAL + 512;
    assert(rbtree_delete(&tree, &i) == 0);
    rb_assert(tree.root);

    teardown();
}

void test_rbtree_delete_all() {

    int i;

    setup(0);

    for(i=0; i < NODES; i++) {

        if (keyref_exists(0, i-1, keyref[i]))
            continue;
        
        assert(rbtree_delete(&tree, &keyref[i]));
        rb_assert(tree.root);
    }

    assert(rbtree_is_empty(&tree));
    rb_assert(tree.root);

    teardown();
}

void test_rbtree_walk() {

    setup(1);

    rbtree_walk(&tree, walk_fn);
    rb_assert(tree.root);

    teardown();
}

void test_rbtree_search() {

    int s, *f;

    setup(0);

    s = keyref[NODES/2];
    f = rbtree_search(&tree, &s);
    rb_assert(tree.root);
    assert(f && *f == s);

    s = MAX_VAL + 512;
    f = rbtree_search(&tree, &s);
    rb_assert(tree.root);
    assert(f == NULL);

    teardown();
}

int main() {

    test_rbtree_delete();
    test_rbtree_delete_all();
    test_rbtree_search();
    test_rbtree_is_empty();
    test_rbtree_walk();

    return 0;
}
