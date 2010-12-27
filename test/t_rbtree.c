
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

#define MAX_VAL 500
#define NODES   20

#define is_red(n) ((n) != NULL && (n)->color == RB_RED)

static int vcmp(const void *a, const void *b);
static void vdelete(void *ptr);

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

/* data */
static rbtree tree = RBTREE_INIT(vdelete, NULL, vcmp);
static int   keyref[NODES];

static int vcmp(const void *a, const void *b) {

    return *((int*)a) - *((int*)b);
}

static void vdelete(void *ptr) {

    int i;

    printf("delete: %i\n", *((int*)ptr));
 
    for(i=0; i < NODES; i++) {

        if (keyref[i] == -1)
            continue;

        if (ptr == &keyref[i]) {
            keyref[i] = -1;
            break;
        }
    }
}

static void vwalk(rbnode *node) {

    int i, found = 0;
    
    /* check if this node exist in the reference list */
    for(i=0; i < NODES; i++) {
        
        if (node->key == &keyref[i]) {
            found = 1;
            break;
        }
    }
    assert(found);
}

void test_insert() {

    int i = 0;
    
    /* insert values */
	while(i < NODES) {

        rbnode *node;
        
		keyref[i] = (uint) (rand() % MAX_VAL);
		
        /* insert into rbtree and assert it */
		node = rbtree_insert(&tree, &keyref[i]);
        rb_assert(tree.root);

        assert(node);

        printf("insert: %i\n", *((int*)node->key));
        
        i++;
	}

    rb_assert(tree.root);
}

void test_delete() {

    int i, key;

    /* remove some values */
	for(i=0; i < 10; i++) {

        do
            key = keyref[rand() % NODES];
        while(key < 0);

	    rbtree_delete(&tree, &key);
        rb_assert(tree.root);

        keyref[key] = -1;
	}
}

void test_search() {

    int index;

    int *n;

    do
        index = rand() % NODES;
    while(keyref[index] < 0);

    printf("search: expecting to find key %i\n", keyref[index]);

    /* search for a key we know exists */
    n = rbtree_search(&tree, &keyref[index]);
    rb_assert(tree.root);

    assert(n == &keyref[index]);

    index = MAX_VAL + 512;
    
    printf("search: expecting to not find key: %i\n", index);

    /* search for a key we now don't exist */
    n = rbtree_search(&tree, &index);
    rb_assert(tree.root);

    assert(n == NULL);
}

void test_walk() {

    rbtree_walk(&tree, vwalk);
    rb_assert(tree.root);
}

int main(int argc, char **argv) {
	
	tree.root = NULL;

    utest_init_RNG();
	
    /* a new tree is empty */
	assert(rbtree_is_empty(&tree));

    test_insert();
    
    test_search();
    test_walk();
    
    test_delete();
    
    test_search();
    test_walk();

    /* free the tree */
    rbtree_free(&tree);
    
	/* tree is now empty again */
	assert(rbtree_is_empty(&tree));

    printf("-- TEST PASS --\n");

	return 0;
}
