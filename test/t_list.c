
#include <assert.h>
#include <string.h>

#include "../src/list.h"

int itemcmp(const void *a, const void *b) {

    return strcmp(a, b);
}

void test_insert() {

    int i;
    char ref[4] = { 'a', 'b', 'c', 'd' };
    struct list *l = list_create();

    for(i=0; i < 4; i++)
        list_insert(l, &ref[i]);
    for(i=0; i < 4; i++)
        assert(l->items[i] == &ref[i]);

    list_destroy(l);
}

void test_remove() {

    int i;
    char ref[4] = { 'a', 'b', 'c', 'd' };
    struct list *l = list_create();

    for(i=0; i < 4; i++)
        list_insert(l, &ref[i]);

    list_remove(l, 1);
    list_remove(l, 2);

    assert(list_size(l) == 2);
    assert(l->items[0] == &ref[0]);
    assert(l->items[1] == &ref[2]);

    list_remove(l, 0);
    list_remove(l, 0);

    assert(list_size(l) == 0);
    list_destroy(l);
}


void test_isempty() {

    struct list *l = NULL;

    assert(list_isempty(l));
    l = list_create();
    assert(list_isempty(l));
    list_insert(l, NULL);
    assert(list_isempty(l) == 0);

    list_destroy(l);
}

void test_size() {

    struct list *l = NULL;

    assert(list_size(l) == 0);
    l = list_create();
    assert(list_size(l) == 0);
    list_insert(l, NULL);
    list_insert(l, NULL);
    assert(list_size(l) == 2);
    
    list_destroy(l);
}

void test_indexof() {

    int i;
    char ref[4] = { 'a', 'b', 'c', 'd' };
    struct list *l = list_create();

    assert(list_indexof(l, &ref[2]) == -1);

    for(i=0; i < 4; i++)
        list_insert(l, &ref[i]);

    for(i=0; i < 4; i++)
        assert(list_indexof(l, &ref[i]) == i);

    list_destroy(l);
}

void test_has() {

    int i;
    char *ref[4] = { "a", "b", "c", "d" };
    struct list *l = list_create();

    for(i=0; i < 4; i++)
        list_insert(l, ref[i]);

    assert(list_has(l, "b", itemcmp));
    assert(list_has(l, "e", itemcmp) == 0);

    list_destroy(l);
}

void test_lookup() {

    int i;
    char *ref[4] = { "a", "b", "c", "d" };
    struct list *l = list_create();

    for(i=0; i < 4; i++)
        list_insert(l, ref[i]);

    assert(list_lookup(l, "b", itemcmp) == ref[1]);
    assert(list_lookup(l, "e", itemcmp) == NULL);

    list_destroy(l);
}

int main() {

    test_insert();
    test_remove();
    test_isempty();
    test_size();
    test_indexof();
    test_has();
    test_lookup();

    return 0;
}
