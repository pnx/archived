/* inotify.c - inotify implementation
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik.hautakoski@gmail.com>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@unknown.org>
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

#include "util.h"
/* red black tree for watch descriptors */
#include "rbtree.h"
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

/* redblack tree */
static rbtree tree = RBTREE_INIT(free, NULL, strcmp);

static queue_t event_queue;

static int addwatch(const char *path, const char *name, unsigned recursive) {
	
	char   *npath;
	int     wd;
    
	npath = path_normalize(path, name, 1);

    if (!npath)
        return -1;
    
	wd = inotify_add_watch(fd, npath, WATCH_MASK);
	
	if (wd < 0) {
        free(npath);
		return -1;
	}

    rbtree_insert(&tree, wd, npath);

    logmsg(LOG_DEBUG, "added wd = %i on %s", wd, npath);

    if (recursive) {
        fscrawl_t f = fsc_open(npath);

        if (!f)
            return -1;
        
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

            queue_enqueue(event_queue, ev);
            
            if (ent->dir)
                addwatch(ent->base, ent->name, 0);
        }
    }
	
	return 1;
}

static int rmwatch(const char *path, const char *name) {

    char *fpath;
    rbnode *node;

    fpath = path_normalize(path, name, 1);

    if (!fpath)
        return -1;

    node = rbtree_cmp_search(&tree, fpath);
	
	if (node == NULL) {
		logmsg(LOG_DEBUG, "remove watch: can't find %s", fpath);
        free(fpath);
		return -1;
	}
    free(fpath);
	
	logmsg(LOG_DEBUG, "remove watch: %i %s", node->key, (char*) node->data);

    return inotify_rm_watch(fd, node->key);
}

static void proc_event(inoev *iev) {

	rbnode *node;
    notify_event *event;
	uint8_t type = NOTIFY_UNKNOWN;
	
    logmsg(LOG_DEBUG, "RAW EVENT: %i, %x, %s", iev->wd, iev->mask, iev->name);

    /* this event is triggered when a watch descriptor is removed.
       so we can do a binary search instead of useing the IN_DELETE
       event (that may be triggered on a parent wd) to do a traverse search */
    if (iev->mask & IN_IGNORED) {
        rbtree_delete(&tree, iev->wd);
        return;
    }
    
	/* lookup the watch descriptor in rbtree */
	node = rbtree_search(&tree, iev->wd);
	
	if (!node) {
		logmsg(LOG_WARN, "-- IGNORING EVENT -- invalid watchdescriptor %i", iev->wd);
		return;
	}

	event = notify_event_new();
	
	if (!event)
		return;
	
	/* set dir and drop that bit off (so its not in the way) */
	event->dir = (iev->mask & IN_ISDIR) != 0;
	iev->mask &= ~IN_ISDIR;
	
	notify_event_set_path(event, node->data);
	notify_event_set_filename(event, iev->name);

    /* queue event before doing any fscrawl on a subdirectory
       to prevent messing up the order */
    queue_enqueue(event_queue, event);
    
	switch(iev->mask) {
    case IN_CREATE :			
        if (event->dir) {
            logmsg(LOG_DEBUG, "IN_CREATE on directory, adding");
            addwatch(event->path, event->filename, 0);
        }
        type = NOTIFY_CREATE;
        break;
    case IN_MOVED_TO :
        if (event->dir) {
            logmsg(LOG_DEBUG, "IN_MOVED_TO on directory, adding");
            addwatch(event->path, event->filename, 1);
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
    
	rbtree_free(&tree);
	
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
    return (addwatch(path, NULL, 1) > 0) ? 1 : -1;
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

static void print_node(rbnode *node) {
	printf("%i: %s\n", node->key, (char*) node->data);
}

void notify_stat() {

    if (!init)
        die("inotify is not instantiated.");

	rbtree_walk(&tree, print_node);
}
