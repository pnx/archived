
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

static int vcmp(const char *a, const char *b);
static void vfree(void *ptr);
static void vupdate(void *old, void *new);

/* data */
static rbtree tree = RBTREE_INIT(vfree, vupdate, vcmp);
static int   keyref[NODES];
static char *dataref[NODES];

static int vcmp(const char *a, const char *b) {

    printf("cmp: %s -> %s\n", a, b); 
    
    return strcmp(a, b);
}

static void vupdate(void *old, void *new) {

    int i;

    printf("update: %s -> %s\n", (char*)old, (char*)new);

    for(i=0; i < NODES; i++) {

        if (keyref[i] == -1)
            continue;
        
        if (old == dataref[i]) {
            dataref[i] = new;
            break;
        }
    }
}

static void vfree(void *ptr) {

    int i;

    printf("free: %s\n", (char*)ptr);

    for(i=0; i < NODES; i++) {

        if (keyref[i] == -1)
            continue;
        
        if (ptr == dataref[i]) {
            keyref[i]  = -1;
            dataref[i] = NULL;
            break;
        }
    }

    free(ptr);
}

static void vwalk(rbnode *node) {

    int i, found = 0;
    
    /* check if this node exist in the reference list */
    for(i=0; i < NODES; i++) {
        
        if (node->key == keyref[i] && node->data == dataref[i]) {
            found = 1;
            break;
        }
    }

    assert(found);
}

void test_insert() {

    int i = 0, ret;
    uint ckey;
    char *data;

    utest_init_RNG();

    /* insert values */
	while(i < NODES) {
		
		ckey = (uint) (rand() % MAX_VAL);
        data = utest_ran_string(16);
		
        /* insert into rbtree and assert it */
		ret = rbtree_insert(&tree, ckey, data);
        rb_assert(tree.root);

        if (!ret)
            continue;
        
        printf("insert: %i %s\n", ckey, data);
        
		keyref[i] = ckey;
        dataref[i] = data;
        
        i++;
	}

    /* insert duplicate key */
    rbtree_insert(&tree, keyref[rand() % NODES], strdup("---- update ----"));
    rb_assert(tree.root);
}

void test_delete() {

    int i, key;

    /* remove some values */
	for(i=0; i < 10; i++) {

        do
            key = keyref[rand() % NODES];
        while(key < 0);

	    rbtree_delete(&tree, key);
        rb_assert(tree.root);
	}
}

void test_search() {

    int index;

    rbnode *n;

    do
        index = rand() % NODES;
    while(keyref[index] < 0);

    printf("search: expecting to find key %i\n", keyref[index]);

    /* search for a key we know exists */
    n = rbtree_search(&tree, keyref[index]);
    rb_assert(tree.root);

    assert(n != NULL);
    assert(n->key == keyref[index]);
    assert_string(n->data, dataref[index]);

    printf("search: expecting to not find key: %i\n", MAX_VAL+512);

    /* search for a key we now don't exist */
    n = rbtree_search(&tree, MAX_VAL+512);
    rb_assert(tree.root);

    assert(n == NULL);
}

void test_cmp_search() {

    char *search;
    rbnode *n;

    do
        search = dataref[rand()%NODES];
    while(search == NULL);

    printf("cmp_search: searching for %s\n", search);
    
    n = rbtree_cmp_search(&tree, search);
    
    rb_assert(tree.root);
    assert(n != NULL);
    assert_string(n->data, search);
}

void test_walk() {

    rbtree_walk(&tree, vwalk);
    rb_assert(tree.root);
}

int main(int argc, char **argv) {
	
	tree.root = NULL;
	
    /* a new tree is empty */
	assert(rbtree_is_empty(&tree));

    test_insert();
    
    test_search();
    test_cmp_search();
    test_walk();
    
    test_delete();
    
    test_search();
    test_cmp_search();
    test_walk();

    /* free the tree */
    rbtree_free(&tree);
    
	/* tree is now empty again */
	assert(rbtree_is_empty(&tree));

    printf("-- TEST PASS --\n");

	return 0;
}
