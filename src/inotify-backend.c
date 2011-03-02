/* inotify-backend.c
 *
 *  Copyright (C) 2010-2011   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#include "inotify-backend.h"
#include "inotify-syscalls.h"
#include "log.h"

#define WATCH_MASK (IN_MOVE | IN_CREATE | IN_DELETE | IN_ONLYDIR)

static int fd = -1;

static unsigned inotify_qsize = 256;

int inotify_backend_init(void) {

    if (fd >= 0)
        close(fd);

    fd = inotify_init();
    return fd;
}

void inotify_backend_exit(void) {

    if (fd >= 0)
        close(fd);
    fd = -1;
}

int inotify_backend_watch(const char *path) {

    int wd = inotify_add_watch(fd, path, WATCH_MASK);
    
	if (wd < 0) {
        if (errno != EACCES && errno != ENOTDIR)
            logerrno(LOG_CRIT, "inotify_watch", errno);
		return -errno;
	}
    return wd;
}

int inotify_backend_ignore(int wd) {

    if (inotify_rm_watch(fd, wd) < 0) {
        logerrno(LOG_CRIT, "intotify_ignore", errno);
        return -errno;
    }
    return 0;
}

#define MAX_PENDING 5
#define THROTTLE_SLEEP_NS 2000000
#define UPPER_BOUND(size) ((unsigned) (size) >> 1)

int inotify_backend_read(void *buf, size_t size) {

    struct timespec tres = { 0, THROTTLE_SLEEP_NS };
    unsigned short tcount;
    unsigned int ioready = 0;

    for(tcount = 0; tcount < MAX_PENDING; tcount++) {

        unsigned int events;

        if (ioctl(fd, FIONREAD, &ioready) < 0)
            break;

        events = ioready / IN_EVENT_SIZE;
        if (events > UPPER_BOUND(inotify_qsize))
            goto do_read;

        nanosleep(&tres, NULL);
    }

    if (ioready)
        goto do_read;

    return 0;
do_read:
    return read(fd, buf, size);
}
