/* inotify.c - inotify implementation
 *
 *  Copyright (C) 2010-2011   Henrik Hautakoski <henrik@fiktivkod.org>
 *  Copyright (C) 2010        Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "inotify-backend.h"
#include "inotify-map.h"
#include "inotify-watch.h"
#include "xalloc.h"
#include "util.h"
#include "log.h"
#include "path.h"
#include "queue.h"
#include "fscrawl.h"
#include "inotify.h"
#include "notify.h"

static int init;

static queue_t init_ev_q;

static int addwatch(const char *path, const char *name) {

    fscrawl_t f;
    char *npath = path_normalize(path, name, 1);

    if (!npath)
        return -1;

    if (inotify_watch(npath) < 0)
        goto clean;

    f = fsc_open(npath);
    if (!f)
        goto clean;

    for(;;) {
        notify_event *ev;
        fs_entry *ent = fsc_read(f);

        if (!ent)
            break;

        ev = notify_event_new();

        notify_event_set_type(ev, NOTIFY_CREATE);
        notify_event_set_path(ev, ent->base);
        notify_event_set_filename(ev, ent->name);
        notify_event_set_dir(ev, ent->dir);

        if (ev->dir) {
            char *fullpath = path_normalize(ev->path, ent->name, 1);
            if (fullpath && inotify_watch(fullpath) < 0)
                xfree(fullpath);
        }

        queue_enqueue(init_ev_q, ev);
    }
    fsc_close(f);

    return 1;
clean:
    xfree(npath);
    return -1;
}

static int rmwatch(const char *path, const char *name) {

    char *fpath;
    int ret;

    fpath = path_normalize(path, name, 1);

    if (!fpath)
        return -1;

    ret = inotify_ignore(fpath);
    free(fpath);
    return ret;
}

int notify_init() {

    if (!init) {
        if (inotify_init() < 0)
            return -1;
        init_ev_q = queue_init();
        init = 1;
    }
    return 0;
}

void notify_exit() {

    if (init) {
        notify_event *e;
        while((e = queue_dequeue(init_ev_q)))
            notify_event_del(e);
             queue_destroy(init_ev_q);
        init_ev_q = NULL;

        inotify_exit();

        init = 0;
    }
}

/*
 * Add inotify watch on path
 */
int notify_add_watch(const char *path) {

    if (!init)
        die("inotify is not instantiated.");
    return (addwatch(path, NULL) > 0) ? 1 : -1;
}

int notify_rm_watch(const char *path) {

    if (!init)
        die("inotify is not instantiated.");
    return rmwatch(path, NULL);
}

notify_event* notify_read() {

    notify_event *ev;

    if (!init)
        die("inotify is not instantiated.");

    if (!queue_isempty(init_ev_q))
        return queue_dequeue(init_ev_q);

    ev = inotify_read();

    if (ev && ev->dir && ev->type == NOTIFY_CREATE)
        addwatch(ev->path, ev->filename);

    return ev;
}

void notify_stat() {

    if (!init)
        die("inotify is not instantiated.");
}
