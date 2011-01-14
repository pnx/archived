
#include <assert.h>
#include <stdlib.h>
#include "../src/inotify-map.h"

int   wdref[4]   = { 1, 2, 1, 4 };
char *pathref[4] = { "one", "two", "three", "four" };

static void setup() {

    int i;

    if (!inotify_map_isempty())
        return;

    for(i=0; i < 4; i++)
        inotify_map(wdref[i], pathref[i]);
}

static void teardown() {

    inotify_unmap_all();
}

static void validate_list(int refindex, const struct str_list *l) {

    int i;

    assert(l);

    for(i=0; i < 4; i++) {
            
        if (i != wdref[refindex])
            continue;

        assert(str_list_has(l, pathref[refindex]));
    }
}

void test_inotify_unmap_wd() {

    int i;

    setup();

    assert(inotify_unmap_wd(wdref[0]));
    assert(inotify_unmap_wd(wdref[1]));
    assert(inotify_unmap_wd(wdref[2]) == 0);
    assert(inotify_unmap_wd(wdref[3]));

    assert(inotify_map_isempty());

    teardown();
}

void test_inotify_unmap_path() {

    int i;

    setup();

    for(i=0; i < 4; i++)
        assert(inotify_unmap_path(pathref[i]));

    assert(inotify_map_isempty());

    teardown();
}

void test_inotify_unmap_all() {

    setup();
    
    inotify_unmap_all();
    assert(inotify_map_isempty());

    teardown();
}

void test_inotify_map_lookup() {

    int i;

    setup();

    for(i=0; i < 4; i++)
        validate_list(i, inotify_map_lookup(wdref[i]));

    assert(inotify_map_lookup(25) == NULL);
    
    teardown();
}

void test_inotify_map_lookup_by_path() {

    int i;

    setup();
    
    for(i=0; i < 4; i++)
        validate_list(i, inotify_map_lookup_by_path(pathref[i]));

    assert(inotify_map_lookup(33) == NULL);
    
    teardown();
}

int main() {

    test_inotify_unmap_wd();
    test_inotify_unmap_path();
    test_inotify_unmap_all();
    test_inotify_map_lookup();
    test_inotify_map_lookup_by_path();
    
    return 0;
}
