#ifndef RB_DEBUG
    #define RB_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "../src/common/rbtree.h"

#define MAX_VAL 12500
#define NODES 5000

uint get_random_key(uint m) {
	return (uint) (rand() % m);
}

void printer(rbnode *node) {
	
	printf("node: %u (%p)\n", node->key, node);
}

int main(int argc, char **argv) {
	
	uint keyref[NODES];
	uint i, spos, search_key, ckey;
	rbtree tree;
	rbnode *snode;
	
	tree.root = NULL;
	
	srand(time(NULL));
	
	/* rbtree should be empty */
	assert(rbtree_is_empty(&tree) == 1);
	
	/* get a random node position */
	spos = get_random_key(NODES);
	
	/* insert values */
	for(i=0; i < NODES; i++) {
		
		ckey = get_random_key(MAX_VAL);
		
		if(i == spos)
			search_key = ckey;
		
		//printf("insert: %u\n", ckey);
		rbtree_insert(&tree, ckey, NULL);
		keyref[i] = ckey;
	}
	
	/* validate tree */
	rb_assert(tree.root);
	
	/* search the random key */
	snode = rbtree_search(&tree, search_key);
	
	assert(snode->key == search_key);

    /* remove some values */	
	for(i=0; i < NODES; i++) {
	
		//printf("removing: %u\n", ckey);
		
	    rbtree_delete(&tree, ckey);
	    
	    ckey = get_random_key(MAX_VAL);
	}
	
    /* assert again */
    rb_assert(tree.root);

    if(argc == 1) {
    	/* free the tree */
    	rbtree_free(&tree, NULL);
    } else {
    	printf("---\n");
    	for(i=0; i < NODES; i++) {
    		//printf("removing: %u\n", keyref[i]);
    		rbtree_delete(&tree, keyref[i]);
    	}
    }
	
    rbtree_walk(&tree, printer);
    
	/* tree should now be empty */
	assert(rbtree_is_empty(&tree) == 1);
	
	return 0;
}
