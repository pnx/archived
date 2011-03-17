
#ifndef INOTIFY_WATCH_H
#define INOTIFY_WATCH_H

#include "tree.h"

struct watch {
    struct tree tree;
    int wd;
    const char *path;
};

struct watch* inotify_watch_new(int wd, const char *path);

void inotify_watch_destroy(struct watch *watch, void (*fn)(struct watch *));

struct watch* inotify_watch_add(struct watch *parent, struct watch *watch);

void inotify_watch_rm(struct watch *watch);

struct watch* inotify_watch_find_child(struct watch *watch, const char *path);

#endif
