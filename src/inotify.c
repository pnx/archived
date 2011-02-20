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
#include <time.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>

#include "inotify-map.h"
#include "inotify-watch.h"
#include "util.h"
#include "log.h"
#include "path.h"
#include "queue.h"
#include "fscrawl.h"
#include "notify.h"

typedef struct inotify_event inoev;

#define INOBUFSIZE ((1 << 12) * (sizeof(inoev) + 0x40))

#define WATCH_MASK (IN_MOVE | IN_CREATE | IN_DELETE | IN_ONLYDIR)

static int init = 0;

/* Inotify file descriptor */
static int fd;

static queue_t event_queue;

static int inotify_watch(const char *path) {

    int wd = inotify_add_watch(fd, path, WATCH_MASK);
	
	if (wd < 0) {
        if (errno != EACCES)
            logerrno(LOG_CRIT, "inotify_watch", errno);
		return -1;
	}

    inotify_map(wd, path);

    return wd;
}

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

        if (ent->dir) {
            char *fullpath = path_normalize(ev->path, ev->filename, 1);
            if (fullpath) {
                if (inotify_watch(fullpath) < 0) {
                    xfree(fullpath);
                }
            }
        }

        queue_enqueue(event_queue, ev);
    }
    fsc_close(f);

    return 1;
clean:
    xfree(npath);
    return -1;
}

static int rmwatch(const char *path, const char *name) {

    char *fpath;
    int wd;

    fpath = path_normalize(path, name, 1);

    if (!fpath)
        return -1;

    wd = inotify_map_get_wd(fpath);
    
	if (wd <= 0) {
		logmsg(LOG_DEBUG, "remove watch: can't find %s", fpath);
        free(fpath);
		return -1;
	}
    
	logmsg(LOG_DEBUG, "remove watch: %i %s", wd, fpath);
    free(fpath);

    if (inotify_rm_watch(fd, wd) < 0) {
        logerrno(LOG_CRIT, "intotify_rm_watch", errno);
        return -1;
    }
    return 0;
}

static void proc_event(inoev *iev) {

    int i;
    struct list *watch_list;
    int isdir;
    
    logmsg(LOG_DEBUG, "RAW EVENT: %i, %x, %s", iev->wd, iev->mask, iev->name);

    if (iev->mask & IN_IGNORED) {
        inotify_unmap_wd(iev->wd);
        return;
    }
    
	/* lookup watch descriptors */
	watch_list = inotify_map_get_path(iev->wd);
	
	if (!watch_list) {
		logmsg(LOG_WARN, "-- IGNORING EVENT -- invalid watchdescriptor %i", iev->wd);
		return;
	}

    /* set dir and drop that bit off (so its not in the way) */
    isdir = (iev->mask & IN_ISDIR) != 0;
    iev->mask &= ~IN_ISDIR;

    for(i=0; i < watch_list->nr; i++) {
        
        uint8_t type = NOTIFY_UNKNOWN;
        struct watch *watch = watch_list->items[i];
        notify_event *event = notify_event_new();

        notify_event_set_path(event, watch->path);
        notify_event_set_filename(event, iev->name);
        event->dir = isdir;

        /* queue event before doing any fscrawl on a subdirectory
           to prevent messing up the order */
        queue_enqueue(event_queue, event);
    
        switch(iev->mask) {
        case IN_CREATE :			
            if (event->dir) {
                logmsg(LOG_DEBUG, "IN_CREATE on directory, adding");
                addwatch(event->path, event->filename);
            }
            type = NOTIFY_CREATE;
            break;
        case IN_MOVED_TO :
            if (event->dir) {
                logmsg(LOG_DEBUG, "IN_MOVED_TO on directory, adding");
                addwatch(event->path, event->filename);
            }
            type = NOTIFY_CREATE;
            break;
        case IN_MOVED_FROM :
            if (event->dir)
                rmwatch(event->path, event->filename);
        case IN_DELETE :
            type = NOTIFY_DELETE;
            break;
        }
	
        event->type = type;
    }

    list_destroy(watch_list);
}

int notify_init() {

	if (init)
		return 0;
	
	fd = inotify_init();
    
	if (fd < 0)
		return -1;

	event_queue = queue_init();
	
	if (event_queue == NULL)
		return -1;
        
    init = 1;
	return 0;
}

void notify_exit() {

    if (!init)
        return;
        
    close(fd);

    inotify_unmap_all();
	
	if (event_queue) {
        notify_event *e;
        while(e = queue_dequeue(event_queue))
            notify_event_del(e); 
		queue_destroy(event_queue);
        event_queue = NULL;
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

    /* bytes ready on the inotify descriptor */
    int ioready;

    if (!init)
        die("inotify is not instantiated.");
    
    /* if we don't have pending events, wait for more data on fd  */
    if (queue_isempty(event_queue)) {

        /* time resolution */
        struct timespec tres = { 0, 2000000 };

        unsigned short tcount;
        
        for(tcount = 0; tcount < 10; tcount++) {

            if (ioctl(fd, FIONREAD, &ioready) == -1)
                break;

            if (ioready > INOBUFSIZE)
                break;

            nanosleep(&tres, NULL);
        }
    }
    /* otherwise, only read if the data available at
       this given moment is "large enough" */
    else {
        ioctl(fd, FIONREAD, &ioready);

        if (ioready < INOBUFSIZE / 2)
            ioready = 0;
    }

    while(ioready > 0) {

        char buf[INOBUFSIZE];
        int offset = 0, rbytes = read(fd, buf, INOBUFSIZE);

        logmsg(LOG_DEBUG, "%i bytes avail", ioready);

        if (rbytes == -1) {
            logerrno(LOG_WARN, "INOTIFY", errno);
            break;
        }

        while(rbytes > offset) {
            inoev *rev = (inoev *) &buf[offset];
            proc_event(rev);
            offset += sizeof(inoev) + rev->len;
        }
        ioready -= rbytes;
    }

    return queue_dequeue(event_queue);
}

void notify_stat() {

    if (!init)
        die("inotify is not instantiated.");
}
