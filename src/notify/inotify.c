/* notify/inotify.c - inotify implementation
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
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>

#include "../common/util.h"
/* red black tree for watch descriptors */
#include "../common/rbtree.h"
#include "../common/debug.h"
#include "../common/path.h"

#include "queue.h"
#include "notify.h"

typedef struct inotify_event inoev;

#define INOBUFSIZE ((1 << 12) * (sizeof(inoev) + 0x40))

#define WATCH_MASK (IN_MOVE | IN_CREATE | IN_DELETE | IN_ONLYDIR)

/* Inotify file descriptor */
static int fd = 0;

/* redblack tree */
static rbtree tree;

static queue_t event_queue;

static int addwatch(const char *path, const char *name) {
	
	rbnode *node;
	char   *cpath;
	int     wd;
    
	cpath = path_normalize(path, name, 1);
	
	wd = inotify_add_watch(fd, cpath, WATCH_MASK);
	
	if (wd < 0) {
		perror("NOTIFY ADD");
		dprint("%i path %s\n", errno == EBADF, cpath);
		return -errno;
	}

	/* we must update to not introduce duplicated wd's (keys) */
	node = rbtree_search(&tree, wd);
	
	if (node == NULL) {
		dprint("added wd = %i on %s\n", wd, cpath);
		rbtree_insert(&tree, wd, (void*)cpath);
	} else {
		dprint("updated wd = %i from %s to %s\n", wd, (char*)node->data, cpath);
		free(node->data);
		node->data = (void*) cpath;
	}
	
	return wd;
}

static int rmwatch(unsigned int wd) {
	
	void *data = rbtree_delete(&tree, wd);
	
	if (data == NULL)
		return 0;
	
	dprint("remove watch: %i %s\n", wd, (char*) data);
    inotify_rm_watch(fd, wd);
	free(data);
	return 1;
}

static void proc_event(inoev *iev) {

	rbnode *node;
    notify_event *event;
    char buf[4096];
	uint8_t type = NOTIFY_UNKNOWN;

    event = notify_event_new();
	
#ifdef __DEBUG__
	fprintf(stderr, "RAW EVENT: %i, %x", iev->wd, iev->mask);
	if (iev->len)
		fprintf(stderr, ", %s\n", iev->name);
	else
		fprintf(stderr, "\n");
#endif
	
	/* lookup the watch descriptor in rbtree */
	node = rbtree_search(&tree, iev->wd);
	
	if (node == NULL) {
		dprint("-- IGNORING EVENT -- invalid watchdescriptor %i\n", iev->wd);
		goto cleanup;
	}

	notify_event_set_dir(event, (iev->mask & IN_ISDIR) != 0);
	
	/* set path, this is stored in void* node->data */
	notify_event_set_path(event, (char*)node->data);
	notify_event_set_filename(event, iev->name);
	
	iev->mask &= ~IN_ISDIR;
	
	switch (iev->mask) {
			
		case IN_CREATE :
						
			if (event->dir) {
				dprint("IN_CREATE on directory, adding\n");
				addwatch(event->path, event->filename);
			}
			
			type = NOTIFY_CREATE;
			break;
		case IN_MOVED_TO :
			
			if (event->dir) {
				dprint("IN_MOVED_TO on directory, adding\n");
				addwatch(event->path, event->filename);
			}
            
			type = NOTIFY_MOVE_TO;
			break;
		case IN_MOVED_FROM :
            /* TODO: clean this */
            sprintf(buf, "%s%s/", event->path, event->filename);
            notify_rm_watch(buf);
		case IN_DELETE :
			type = NOTIFY_DELETE;
			break;
		case IN_IGNORED :
			rmwatch(iev->wd);
			goto cleanup;
	}
	
	notify_event_set_type(event, type);
    
	queue_enqueue(event_queue, event);
    return;
cleanup:
    free(event);
}

int notify_init() {

	if (fd > 0) {
		fprintf(stderr, "inotify already instantiated.");
		return 0;
	}
	
	fd = inotify_init();
	
	if (fd < 1) {
		perror("NOTIFY INIT");
		return -1;
	}

	event_queue = queue_init();
	
	if (event_queue == NULL) {
		perror("EVENT QUEUE");
		return -1;
	}
	
	return 1;
}

void notify_cleanup() {

	fd = 0;
    
	/* free rbtree */
	rbtree_free(&tree, NULL);
	
	if (event_queue) {
		queue_destroy(event_queue);
        event_queue = NULL;
    }
}

/*
 * Add inotify watch on path
 */
int notify_add_watch(const char *path) {
	
	return addwatch(path, NULL);
}

int notify_rm_watch(const char *path) {
	
	rbnode *node = rbtree_cmp_search(&tree, (void*)path, strlen(path));
	
	if (node == NULL) {
		dprint("remove watch: can't find %s\n", path);
		return -1;
	}
	
	dprint("remove watch: %i %s\n", node->key, (char*) node->data);
	rmwatch(node->key);
    return 0;
}

notify_event* notify_read() {

    /* bytes ready on the inotify descriptor */
    int ioready;
    
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

        dprint("%i bytes avail\n", ioready);

        char buf[INOBUFSIZE];
        int offset = 0, rbytes = read(fd, buf, INOBUFSIZE);

        if (rbytes == -1)
            die_errno("INOTIFY");

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

#ifdef __DEBUG__
	rbtree_walk(&tree, print_node);
#endif
}
