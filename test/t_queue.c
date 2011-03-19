
#include <assert.h>
#include <stdlib.h>
#include "../src/queue.h"

static int map[512];
static queue_t q = NULL;

static void setup() {

    int i;

    if (!q)
        assert(q = queue_init());

    for(i=0; i < 512; i++)
        queue_enqueue(q, &map[i]);
}

static void tierdown() {

    if (q)
        while(queue_dequeue(q));
}

void test_enqueue() {

    setup();

    assert(queue_num_items(q) == 512);

    tierdown();
}

void test_dequeue() {

    int i, t;

    assert(queue_dequeue(NULL) == NULL);
    assert(queue_dequeue(q) == NULL);

    setup();

    for(i=0; i < 512; i++)
        assert(queue_dequeue(q) == &map[i]);

    tierdown();
}

void test_isempty() {

    setup();

    assert(queue_isempty(q) == 0);

    while(queue_dequeue(q));

    assert(queue_isempty(q));
    assert(queue_isempty(NULL));

    tierdown();
}

void test_num_items() {

    int i;

    assert(queue_num_items(NULL) == 0);
    assert(queue_num_items(q) == 0);

    setup();

    assert(queue_num_items(q) == 512);

    for(i=0; i < 512/3; i++)
        queue_dequeue(q);

    assert(queue_num_items(q) == 512 - 512/3);

    tierdown();
}

int main() {

    int i;

    for(i=0; i < 512; i++)
        map[i] = i;

    test_enqueue();
    test_dequeue();
    test_isempty();
    test_num_items();

    if (q)
        queue_destroy(q);

    return 0;
}
