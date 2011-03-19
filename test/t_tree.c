
#include <assert.h>
#include "../src/tree.h"

void test_link() {

    struct tree nodes[6] = { TREE_INIT };

    tree_link(&nodes[0], &nodes[1]);
    tree_link(&nodes[0], &nodes[2]);
    tree_link(&nodes[0], &nodes[3]);

    tree_link(&nodes[1], &nodes[4]);
    tree_link(&nodes[2], &nodes[5]);

    /* check the root node */
    assert(nodes[0].parent == NULL);
    assert(nodes[0].next == NULL);
    assert(nodes[0].child == &nodes[1]);

    /* move to depth 2 */
    assert(nodes[1].parent == &nodes[0]);
    assert(nodes[2].parent == &nodes[0]);
    assert(nodes[3].parent == &nodes[0]);
    assert(nodes[1].next == &nodes[2]);
    assert(nodes[2].next == &nodes[3]);
    assert(nodes[3].next == NULL);
    assert(nodes[1].child == &nodes[4]);
    assert(nodes[2].child == &nodes[5]);

    /* move to depth 3 */
    assert(nodes[4].parent == &nodes[1]);
    assert(nodes[5].parent == &nodes[2]);
    assert(nodes[4].next == NULL);
    assert(nodes[5].next == NULL);
    assert(nodes[4].child == NULL);
    assert(nodes[5].child == NULL);
}

void test_unlink() {

    struct tree nodes[6] = { TREE_INIT };

    tree_link(&nodes[0], &nodes[1]);
    tree_link(&nodes[0], &nodes[2]);

    tree_link(&nodes[1], &nodes[3]);
    tree_link(&nodes[1], &nodes[4]);
    tree_link(&nodes[2], &nodes[5]);

    /* check the tree */
    assert(nodes[0].parent == NULL);
    assert(nodes[0].next == NULL);
    assert(nodes[0].child == &nodes[1]);

    /* move to depth 2 */
    assert(nodes[1].parent == &nodes[0]);
    assert(nodes[2].parent == &nodes[0]);
    assert(nodes[1].next == &nodes[2]);
    assert(nodes[2].next == NULL);
    assert(nodes[1].child == &nodes[3]);
    assert(nodes[2].child == &nodes[5]);

    /* move to depth 3 */
    assert(nodes[3].parent == &nodes[1]);
    assert(nodes[4].parent == &nodes[1]);
    assert(nodes[5].parent == &nodes[2]);
    assert(nodes[3].next == &nodes[4]);
    assert(nodes[4].next == NULL);
    assert(nodes[5].next == NULL);
    assert(nodes[3].child == NULL);
    assert(nodes[4].child == NULL);
    assert(nodes[5].child == NULL);

    tree_unlink(&nodes[1]);

    /* check the new tree */
    assert(nodes[0].parent == NULL);
    assert(nodes[0].next == NULL);
    assert(nodes[0].child == &nodes[2]);

    /* move to depth 2 */
    assert(nodes[2].parent == &nodes[0]);
    assert(nodes[3].parent == &nodes[0]);
    assert(nodes[4].parent == &nodes[0]);
    assert(nodes[2].next == &nodes[3]);
    assert(nodes[3].next == &nodes[4]);
    assert(nodes[4].next == NULL);
    assert(nodes[2].child == &nodes[5]);
    assert(nodes[3].child == NULL);
    assert(nodes[4].child == NULL);

    /* move to depth 3 */
    assert(nodes[5].parent == &nodes[2]);
    assert(nodes[5].next == NULL);
    assert(nodes[5].child == NULL);
}

void test_detach() {

    struct tree nodes[6] = { TREE_INIT };

    tree_link(&nodes[0], &nodes[1]);
    tree_link(&nodes[0], &nodes[2]);

    tree_link(&nodes[1], &nodes[3]);
    tree_link(&nodes[1], &nodes[4]);
    tree_link(&nodes[2], &nodes[5]);

    /* check the tree */
    assert(nodes[0].parent == NULL);
    assert(nodes[0].next == NULL);
    assert(nodes[0].child == &nodes[1]);

    /* move to depth 2 */
    assert(nodes[1].parent == &nodes[0]);
    assert(nodes[2].parent == &nodes[0]);
    assert(nodes[1].next == &nodes[2]);
    assert(nodes[2].next == NULL);
    assert(nodes[1].child == &nodes[3]);
    assert(nodes[2].child == &nodes[5]);

    /* move to depth 3 */
    assert(nodes[3].parent == &nodes[1]);
    assert(nodes[4].parent == &nodes[1]);
    assert(nodes[5].parent == &nodes[2]);
    assert(nodes[3].next == &nodes[4]);
    assert(nodes[4].next == NULL);
    assert(nodes[5].next == NULL);
    assert(nodes[3].child == NULL);
    assert(nodes[4].child == NULL);
    assert(nodes[5].child == NULL);

    tree_detach(&nodes[1]);

    /* check the tree */
    assert(nodes[0].parent == NULL);
    assert(nodes[0].next == NULL);
    assert(nodes[0].child == &nodes[2]);

    /* move to depth 2 */
    assert(nodes[2].parent == &nodes[0]);
    assert(nodes[2].next == NULL);
    assert(nodes[2].child == &nodes[5]);

    /* move to depth 3 */
    assert(nodes[5].parent == &nodes[2]);
    assert(nodes[5].next == NULL);
    assert(nodes[5].child == NULL);

    /* detached tree */
    assert(nodes[1].parent == NULL);
    assert(nodes[1].next == NULL);
    assert(nodes[1].child == &nodes[3]);

    assert(nodes[3].parent == &nodes[1]);
    assert(nodes[4].parent == &nodes[1]);
    assert(nodes[3].next == &nodes[4]);
    assert(nodes[4].next == NULL);
    assert(nodes[3].child == NULL);
    assert(nodes[4].child == NULL);
}

void test_move() {

    struct tree nodes[7] = { TREE_INIT };

    tree_link(&nodes[0], &nodes[1]);
    tree_link(&nodes[0], &nodes[2]);
    tree_link(&nodes[0], &nodes[3]);

    tree_link(&nodes[1], &nodes[4]);
    tree_link(&nodes[2], &nodes[5]);
    tree_link(&nodes[2], &nodes[6]);

    tree_move(&nodes[5], &nodes[1]);

    assert(nodes[0].parent == NULL);
    assert(nodes[0].next == NULL);
    assert(nodes[0].child == &nodes[2]);

    assert(nodes[3].parent == &nodes[0]);
    assert(nodes[2].parent == &nodes[0]);
    assert(nodes[2].next == &nodes[3]);
    assert(nodes[3].next == NULL);
    assert(nodes[2].child == &nodes[5]);
    assert(nodes[3].child == NULL);

    assert(nodes[5].parent == &nodes[2]);
    assert(nodes[6].parent == &nodes[2]);
    assert(nodes[5].next == &nodes[6]);
    assert(nodes[6].next == NULL);
    assert(nodes[5].child == &nodes[1]);
    assert(nodes[6].child == NULL);

    assert(nodes[1].parent == &nodes[5]);
    assert(nodes[1].next == NULL);
    assert(nodes[1].child == &nodes[4]);

    assert(nodes[4].parent == &nodes[1]);
    assert(nodes[4].next == NULL);
    assert(nodes[4].child == NULL);
}

void test_parent_count() {

    struct tree nodes[6] = { TREE_INIT };

    tree_link(&nodes[0], &nodes[1]);
    tree_link(&nodes[0], &nodes[2]);

    tree_link(&nodes[1], &nodes[3]);
    tree_link(&nodes[1], &nodes[4]);
    tree_link(&nodes[2], &nodes[5]);

    assert(tree_parent_count(&nodes[5]) == 2);
}

int main() {

    test_link();
    test_unlink();
    test_detach();
    test_move();
    test_parent_count();

    return 0;
}
