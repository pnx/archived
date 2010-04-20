/*
 * Copyright (C) 2010  Archived
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>

/* red black tree for watch descriptors */
#include "../common/rbtree.h"
#include "../common/debug.h"
#include "../common/path.h"
#include "notify.h"

typedef struct inotify_event inoev;

#define MAXEVENT     0x200
#define PADEVSIZE    (sizeof(inoev) + 0x40)
#define INOBUFSIZE   (MAXEVENT*PADEVSIZE)

#define WATCH_MASK (IN_MOVE | IN_CREATE | IN_DELETE | IN_ONLYDIR)

/* Inotify file descriptor */
static int fd = 0;

/* Available bytes on file descriptor */
static unsigned int fd_bytes = 0;

/* redblack tree */
static rbtree tree;

/* inotify queue buffer */
static char *inobuf = NULL;

/* current event */
static notify_event event;

static int rmwatch(unsigned int wd) {
	
	void *data = rbtree_delete(&tree, wd);
	
	if (data == NULL)
		return 0;
	
	dprint("remove watch: %i %s\n", wd, (char*) data);
	free(data);
	return 1;
}

static int addwatch(const char *path, const char *name) {
	
	rbnode *node;
	char   *cpath;
	int     wd;
	
	cpath = fmt_path(path, name, 1);
	
	if (cpath == NULL)
		return -1;
	
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

/* REF:1 */
static int proc_event(inoev *iev) {

	rbnode *node;
	char    buf[4096];
	uint8_t type = NOTIFY_UNKNOWN;

	if (iev == NULL)
		return 0;
	
	//notify_event_clear(&event);
	
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
#ifdef __DEBUG__
		fprintf(stderr, "-- IGNORING EVENT -- invalid watchdescriptor %i\n", iev->wd);
		rb_assert(tree.root);
#endif
		return 0;
	}

	notify_event_set_dir(&event, (iev->mask & IN_ISDIR) != 0);
	
	/* set path, this is stored in void* node->data */
	notify_event_set_path(&event, (char*)node->data);
	notify_event_set_filename(&event, iev->name);
	
	iev->mask &= ~IN_ISDIR;
	
	switch (iev->mask) {
			
		case IN_CREATE :
						
			if (event.dir) {
				dprint("IN_CREATE on directory, adding\n");
				addwatch(event.path, event.filename);
			}
			
			type = NOTIFY_CREATE;
			break;
		case IN_MOVED_TO :
			
			if (event.dir) {
				dprint("IN_CREATE on directory, adding\n");
				addwatch(event.path, event.filename);
			}
					
			type = NOTIFY_MOVE_TO;
			break;
		case IN_MOVED_FROM :
			/* TODO: clean this */
			sprintf(buf, "%s%s/", event.path, event.filename);
			notify_rm_watch_path(buf);
		case IN_DELETE :
			type = NOTIFY_DELETE;
			break;
		case IN_IGNORED :
			rmwatch(iev->wd);
			return 0;
	}
	
	notify_event_set_type(&event, type);
	
	return 1;
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
	
	dprint("inotify descriptor = %i\n", fd);

	inobuf = malloc(INOBUFSIZE);
	
	if (inobuf == NULL) {
		perror("NOTIFY BUFFER");
		return -1;
	}
	
	return 1;
}

void notify_cleanup() {
	
	/* free rbtree */
	rbtree_free(&tree, NULL);
	
	if (inobuf != NULL)
		free(inobuf);
	
	fd = 0;
}

/*
 * Add inotify watch on path
 */
int notify_add_watch(const char *path) {
	
	return addwatch(path, NULL);
}

void notify_rm_watch(unsigned int wd) {
	inotify_rm_watch(fd, wd);
}

void notify_rm_watch_path(const char *path) {
	
	rbnode *node = rbtree_cmp_search(&tree, (void*)path, strlen(path));
	
	if (node == NULL) {
		dprint("remove watch by path: can't find %s\n", path);
		return;
	}
	
	dprint("remove watch by path: %i %s\n", node->key, (char*) node->data);
	notify_rm_watch(node->key);
}

/*
 * Get next inotify event
 */
notify_event* notify_read() {
	
	int			rc;
	inoev 	   *rev = NULL;
    static int  offset;

	while(! proc_event(rev)) {
		
		if (fd_bytes < 1) {
		
			offset = 0;

			fd_bytes = read(fd, inobuf, INOBUFSIZE);
		
			if (fd_bytes < 1) {
			
				if (fd_bytes == 0) {
					fprintf(stderr, "NOTIFY READ: EOF\n");
				} 
				/* EINTR = call interrupted, silent ignore */
				else if (errno != EINTR) {
					perror("NOTIFY READ");
				}
			
				return NULL;
			}
		}
	
		rev = (inoev *) &inobuf[offset];
		offset += sizeof(inoev) + rev->len;
	
		if (offset >= fd_bytes)
			fd_bytes = 0;
	}
	
	return &event;
}

static void print_node(rbnode *node) {
	printf("%i: %s\n", node->key, (char*) node->data);
}

void notify_stat() {

#ifdef __DEBUG__
	rbtree_walk(&tree, print_node);
#endif
}

int notify_is_ready() {
	
	unsigned int bytes;
	
	if (fd_bytes > 1)
		return 1;
	
	if (ioctl(fd, FIONREAD, &bytes) == -1)
		return 0;
	
    dprint("%i bytes ready!\n", bytes);
    
	return bytes > 512;
}


