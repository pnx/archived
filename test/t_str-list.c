
#include "../src/str-list.h"
#include "unit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void test_insert() {

    int i;
    char *ref[4] = { "a", "b", "c", "d" };
    struct str_list *l = str_list_create();

    str_list_insert(l, "a");
    str_list_insert(l, "c");
    str_list_insert(l, "b");
    str_list_insert(l, "d");
    
    for(i=0; i < 4; i++)
        assert(strcmp(l->items[i], ref[i]) == 0);
    
    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

void test_remove() {

    char *ref[2] = { "a", "c" };
    struct str_list *l = str_list_create();
    
    str_list_insert(l, "a");
    str_list_insert(l, "c");
    str_list_insert(l, "b");
    str_list_insert(l, "d");
    
    str_list_remove(l, "d");
    str_list_remove(l, "b");
    
    assert(str_list_size(l) == 2);
    assert(strcmp(l->items[0], ref[0]) == 0);
    assert(strcmp(l->items[1], ref[1]) == 0);
    
    str_list_remove(l, "c");
    str_list_remove(l, "a");
    
    assert(str_list_size(l) == 0);
    str_list_destroy(l);
}

void test_isempty() {
    
    struct str_list *l = NULL;
    
    assert(str_list_isempty(l));
    l = str_list_create();
    assert(str_list_isempty(l));
    str_list_insert(l, "string");
    assert(str_list_isempty(l) == 0);
    
    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

void test_size() {

    struct str_list *l = NULL;
    
    assert(str_list_size(l) == 0);
    l = str_list_create();
    assert(str_list_size(l) == 0);
    str_list_insert(l, "a");
    str_list_insert(l, "b");
    assert(str_list_size(l) == 2);
    
    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

void test_indexof() {

    struct str_list *l = str_list_create();

    str_list_insert(l, "a");
    str_list_insert(l, "b");
    str_list_insert(l, "c");

    assert(str_list_indexof(l, "a") == 0);
    assert(str_list_indexof(l, "b") == 1);
    assert(str_list_indexof(l, "c") == 2);

    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

void test_foreach() {

    int i = 0;
    char **item, *ref[4] = { "a", "b", "c", "d" };
    struct str_list *l = str_list_create();

    str_list_insert(l, "a");
    str_list_insert(l, "c");
    str_list_insert(l, "b");
    str_list_insert(l, "d");

    str_list_foreach(item, l) {
        assert_string(ref[i], *item);
        i++; 
    }

    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

void test_has() {

    struct str_list *l = str_list_create();
    
    str_list_insert(l, "something");
    str_list_insert(l, "somethingelse");
    assert(str_list_has(l, l->items[1]));
    assert(str_list_has(l, "don't exists") == 0);
    
    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

void test_lookup() {

    struct str_list *l = str_list_create();

    str_list_insert(l, "something");
    str_list_insert(l, "find me");
    str_list_insert(l, "random");
    
    assert(strcmp(str_list_lookup(l, "find me"), "find me") == 0);
    assert(str_list_lookup(l, "don't exists") == NULL);
    
    str_list_clear_fn(l, NULL);
    str_list_destroy(l);
}

int main() {

    test_insert();
    test_remove();
    test_isempty();
    test_size();
    test_indexof();
    test_foreach();
    test_has();
    test_lookup();

    return 0;
}
