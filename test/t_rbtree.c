
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "unit.h"
#include "../src/rbtree.h"

#define NODES   3000
#define MAX_VAL (3*(NODES/4))

static int vcmp(const void *a, const void *b);
static void vdelete(void *ptr);

/* data */
static rbtree tree = RBTREE_INIT(vcmp);
static int   keyref[NODES];

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

static void walk_fn(const void *key) {

    static int i = 0;

    while(keyref_exists(0, i-1, keyref[i]))
        i++;

    assert(keyref[i] == *((int*)key));

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

    for(i=0; i < NODES; i++)
        rbtree_insert(&tree, &keyref[i]);
}

static void teardown() {

    int i;

    rbtree_free(&tree, vdelete);

    for(i=0; i < NODES; i++)
        keyref[i] = -1;
}

void test_rbtree_is_empty() {

    setup(0);

    assert(rbtree_is_empty(&tree) == 0);

    rbtree_free(&tree, vdelete);

    assert(rbtree_is_empty(&tree));

    teardown();
}

void test_rbtree_delete() {

    int i;

    setup(0);

    for(i=NODES/2; i < 3*(NODES/4); i++) {

        if (keyref_exists(NODES/2, i-1, keyref[i]))
            continue;

        assert(rbtree_delete(&tree, &keyref[i]));
    }

    /* delete a key that does not exist */
    i = MAX_VAL + 512;
    assert(rbtree_delete(&tree, &i) == 0);

    teardown();
}

void test_rbtree_delete_all() {

    int i;

    setup(0);

    for(i=0; i < NODES; i++) {

        if (keyref_exists(0, i-1, keyref[i]))
            continue;

        assert(rbtree_delete(&tree, &keyref[i]));
    }

    assert(rbtree_is_empty(&tree));

    teardown();
}

void test_rbtree_walk() {

    setup(1);

    rbtree_walk(&tree, walk_fn);

    teardown();
}

void test_rbtree_search() {

    int s, *f;

    setup(0);

    s = keyref[NODES/2];
    f = rbtree_search(&tree, &s);
    assert(f && *f == s);

    s = MAX_VAL + 512;
    f = rbtree_search(&tree, &s);
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
