
#include <stdio.h>
#include <string.h>
#include "xalloc.h"
#include "path.h"
#include "inotify-watch.h"

struct watch* inotify_watch_new(int wd, const char *path) {

    struct watch *w = xmallocz(sizeof(struct watch));

    w->wd = wd;
    w->path = path;

    return w;
}

static void watch_free(struct tree *tree, void *data) {

    void (*fn)(struct watch *) = data;

    if (fn)
        fn((struct watch*)tree);
    xfree(tree);
}

void inotify_watch_destroy(struct watch *watch, void (*fn)(struct watch *)) {

    if (watch) {
        struct tree *tree = (struct tree*) watch;

        if (!tree_is_root(tree))
            tree_detach(tree);

        tree_traverse(tree, watch_free, fn);
    }
}

struct watch* inotify_watch_add(struct watch *parent, struct watch *watch) {

    if (parent->tree.child) {
        struct watch *it = (struct watch*)parent->tree.child;

        /* move nodes that are children of this watch alongside watch */
        while(it) {
            if (path_isparent(it->path, watch->path)) {
                struct watch *next = (struct watch*)it->tree.next;
                tree_move(&watch->tree, &it->tree);
                it = next;
            } else {
                it = (struct watch*)it->tree.next;
            }
        }
    }
    return (struct watch*)tree_link(&parent->tree, &watch->tree);
}

void inotify_watch_rm(struct watch *watch) {

    tree_unlink(&watch->tree);
}

struct watch* inotify_watch_find_child(struct watch *watch, const char *path) {

    if (watch->tree.child) {
        struct watch *it = (struct watch *) watch->tree.child;

        while(it) {
            if (!strcmp(watch->path, it->path))
                return it;
            it = (struct watch*) it->tree.next;
        }
    }
    return NULL;
}
