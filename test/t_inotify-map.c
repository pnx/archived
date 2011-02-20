
#include <assert.h>
#include <stdlib.h>
#include "../src/inotify-map.h"

int   wdref[4]   = { 1, 2, 1, 4 };
char *pathref[4] = { "/one/", "/two/", "/three/", "/two/subdir/another/" };

static void setup() {

    int i;

    if (!inotify_map_isempty())
        return;

    for(i=0; i < 4; i++)
        inotify_map(wdref[i], pathref[i]);
}

static void teardown() {

    inotify_unmap_all();
    assert(inotify_map_isempty());
}

static int cmp(const void *a, const void *b) {

    struct watch *w = (struct watch *)a;

    return strcmp(w->path, b);
}

static void validate_list(int index, struct list *list) {

    int i;

    assert(list);

    for(i=0; i < 4; i++) {

        if (i != wdref[index])
            continue;
            
        assert(list_has(list, pathref[index], cmp));
    }
}

void test_inotify_unmap_wd() {

    int i;

    setup();

    assert(inotify_unmap_wd(wdref[0]));
    assert(inotify_unmap_wd(wdref[1]));
    assert(inotify_unmap_wd(wdref[2]) == 0);
    assert(inotify_unmap_wd(wdref[3]) == 0);

    assert(inotify_map_isempty());

    teardown();
}

void test_inotify_unmap_path() {

    setup();

    assert(inotify_unmap_path(pathref[0]) == 1);
    assert(inotify_unmap_path(pathref[1]) == 0);
    assert(inotify_unmap_path(pathref[2]) == 0);
    assert(inotify_unmap_path(pathref[3]) == 0);

    assert(inotify_map_isempty());

    teardown();
}

void test_inotify_unmap_all() {

    setup();
    
    inotify_unmap_all();
    assert(inotify_map_isempty());

    teardown();
}

void test_inotify_map_get_wd() {

    int i;

    setup();
    
    for(i=0; i < 4; i++)
        assert(inotify_map_get_wd(pathref[i]) == wdref[i]);

    teardown();
}

void test_inotify_map_get_path() {

    int i;

    setup();

    for(i=0; i < 4; i++)
        validate_list(i, inotify_map_get_path(wdref[i]));

    assert(inotify_map_get_path(25) == NULL);
    
    teardown();
}

int main() {

    test_inotify_unmap_path();
    test_inotify_unmap_wd();
    test_inotify_unmap_all();
    test_inotify_map_get_wd();
    test_inotify_map_get_path();
    
    return 0;
}
