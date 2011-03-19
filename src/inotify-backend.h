/* inotify-backend.h
 *
 *  Copyright (C) 2005, 2006, 2008, 2009 Free Software Foundation, Inc.
 *  Copyright (C) 2010-2011   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef __INOTIFY_BACKEND
#define __INOTIFY_BACKEND

#include <stdint.h>

/* Structure describing an inotify event.  */
struct inotify_event {
    int wd;                 /* Watch descriptor.  */
    uint32_t mask;          /* Watch mask.  */
    uint32_t cookie;        /* Cookie to synchronize two events.  */
    uint32_t len;           /* Length (including NULs) of name.  */
    char name __flexarr;    /* Name.  */
};

#define IN_EVENT_SIZE (sizeof(struct inotify_event) + 0x40)

/* Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH.  */
#define IN_ACCESS        0x00000001 /* File was accessed.  */
#define IN_MODIFY        0x00000002 /* File was modified.  */
#define IN_ATTRIB        0x00000004 /* Metadata changed.  */
#define IN_CLOSE_WRITE   0x00000008 /* Writtable file was closed.  */
#define IN_CLOSE_NOWRITE 0x00000010 /* Unwrittable file closed.  */
#define IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* Close.  */
#define IN_OPEN          0x00000020 /* File was opened.  */
#define IN_MOVED_FROM    0x00000040 /* File was moved from X.  */
#define IN_MOVED_TO      0x00000080 /* File was moved to Y.  */
#define IN_MOVE          (IN_MOVED_FROM | IN_MOVED_TO) /* Moves.  */
#define IN_CREATE        0x00000100 /* Subfile was created.  */
#define IN_DELETE        0x00000200 /* Subfile was deleted.  */
#define IN_DELETE_SELF   0x00000400 /* Self was deleted.  */
#define IN_MOVE_SELF     0x00000800 /* Self was moved.  */

/* Events sent by the kernel.  */
#define IN_UNMOUNT       0x00002000 /* Backing fs was unmounted.  */
#define IN_Q_OVERFLOW    0x00004000 /* Event queued overflowed.  */
#define IN_IGNORED       0x00008000 /* File was ignored.  */

/* Helper events.  */
#define IN_CLOSE         (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)    /* Close.  */
#define IN_MOVE          (IN_MOVED_FROM | IN_MOVED_TO)          /* Moves.  */

#define IN_ISDIR         0x40000000	/* Event occurred against dir.  */

int inotify_backend_init(void);

void inotify_backend_exit(void);

int inotify_backend_watch(const char *path);

int inotify_backend_ignore(int wd);

int inotify_backend_read(void *buf, size_t size);

#endif /* __INOTIFY_BACKEND */
