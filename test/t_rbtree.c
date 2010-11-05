
/*
 * rb_assert should be called after every rbtree_* operation (not just only the one's
 * that actualy performs changes to the tree) to ensure the tree is valid after the
 * function is done.
 */

#include <stdio.h>
#include <stdlib.h>
#include "unit.h"
#include "../src/rbtree.h"
#include "../src/debug.h"

#define MAX_VAL 500
#define NODES   20

#define is_red(n) (n != NULL && n->color == RB_RED)

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

/* data */
static rbtree tree;
static uint  keyref[NODES];
static char *dataref[NODES];
static uint search_key = -1;
static char search_data[32];

void test_insert() {

    int i = 0, ret;
    uint ckey;
    char *data;

    utest_init_RNG();

    /* insert values */
	while(i < NODES) {
		
		ckey = (uint) (rand() % MAX_VAL);
        data = utest_ran_string(32);
		
        /* insert into rbtree and assert it */
		ret = rbtree_insert(&tree, ckey, data, 33);
        rb_assert(tree.root);
        
        dprint("INSERT: %i %s\n", ckey, data);
        
        /* ignore duplicate key */
        if (ret == -1) {
            free(data);
            continue;
        }
        
		keyref[i] = ckey;
        dataref[i] = data;
        
		if (i == ((NODES/2))) {
			search_key = ckey;
            memcpy(&search_data, data, 33);
        }

        i++;
	}
    
    /* insert duplicate key */
    assert(rbtree_insert(&tree, search_key, "duplicate", 10) == -1);
    rb_assert(tree.root);
}

void test_delete() {

    int i;
    uint key;
    char *data, *dref;

    /* remove some values */	
	for(i=0; i < 10; i++) {

        key = keyref[(NODES/2)+i];
        dref = dataref[(NODES/2)+i];

	    data = rbtree_delete(&tree, key);
        assert_string(data, dref);
        free(data);
        rb_assert(tree.root);
	}
}

void test_search() {

    rbnode *n;

    /* search for a key we know exists */
    n = rbtree_search(&tree, search_key);
    
    assert(n != NULL);
	assert(n->key == search_key);

    /* search for a key we now don't exist */
    n = rbtree_search(&tree, MAX_VAL+512);

    assert(n == NULL);
}

void test_cmp_search() {

    rbnode *n;

    n = rbtree_cmp_search(&tree, &search_data, 32);

    assert(n != NULL);
    assert_string(n->data, search_data);
}

int main(int argc, char **argv) {
	
	tree.root = NULL;
	
    /* a new tree is empty */
	assert(rbtree_is_empty(&tree) == 1);

    test_insert();
    test_search();
    test_cmp_search();
    test_delete();

    /* free the tree */
    rbtree_free(&tree, NULL);
    
	/* tree is now empty again */
	assert(rbtree_is_empty(&tree) == 1);

    printf("-- TEST PASS --\n");
    
	return 0;
}
