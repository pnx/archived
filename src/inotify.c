
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "path.h"
#include "queue.h"
#include "path.h"
#include "inotify-backend.h"
#include "inotify-map.h"
#include "inotify.h"

static queue_t event_queue;

int inotify_init(void) {

    inotify_backend_init();
    event_queue = queue_init();

    return 1;
}

void inotify_exit(void) {

    inotify_backend_exit();

    if (event_queue) {
        notify_event *e;
        while((e = queue_dequeue(event_queue)))
            notify_event_del(e);
         queue_destroy(event_queue);
        event_queue = NULL;
    }
}

int inotify_watch(const char *path) {

    int wd = inotify_backend_watch(path);
    if (wd > 0)
        inotify_map(wd, path);

    return wd;
}

int inotify_ignore(const char *path) {

    int wd = inotify_map_get_wd(path);

    if (wd <= 0)
        return -1;

    /* unmap and remove watch */
    if (inotify_unmap_path(path) == 0) {
        logmsg(LOG_DEBUG, "remove watch: %i %s", wd, path);

        if (inotify_backend_ignore(wd) < 0)
            return -1;
    }
    return 0;
}

static size_t memadd(void *dest, void *src, size_t msize, size_t size) {

    if (msize >= size)
        msize = size;

    memcpy(dest, src, msize);
    return msize;
}

static const char* strmask(uint32_t mask) {

    static char buf[1024];
    unsigned p = 0;

    if (mask & IN_CREATE)
        p = memadd(buf, ",CREATE", 7, sizeof(buf)-1);
    if (mask & IN_DELETE)
        p += memadd(buf + p, ",DELETE", 7, sizeof(buf)-1 - p);
    if (mask & IN_ISDIR)
        p += memadd(buf + p, ",ISDIR", 6, sizeof(buf)-1 - p);
    if (mask & IN_DELETE_SELF)
        p += memadd(buf + p, ",DELETE_SELF", 12, sizeof(buf)-1 - p);
    if (mask & IN_MOVED_TO)
        p += memadd(buf + p, ",MOVED_TO", 9, sizeof(buf)-1 - p);
    if (mask & IN_MOVED_FROM)
        p += memadd(buf + p, ",MOVED_FROM", 11, sizeof(buf)-1 - p);
    if (mask & IN_IGNORED)
        p += memadd(buf + p, ",IGNORED", 8, sizeof(buf)-1 - p);
    if (mask & IN_UNMOUNT)
        p += memadd(buf + p, ",UNMOUNT", 8, sizeof(buf)-1 - p);

#ifdef __DEBUG__
    if (p >= sizeof(buf)-1)
        die("strmask: buffer overflow\n");
#endif /* __DEBUG__ */

    if (!p)
        p = 1;

    buf[p] = '\0';
    return buf + 1;
}

static void proc_event(struct inotify_event *iev) {

    int i;
    struct list *watch_list;

    logmsg(LOG_DEBUG, "RAW EVENT: %i, %x (%s), %s",
        iev->wd, iev->mask, strmask(iev->mask), iev->name);

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

    for(i=0; i < watch_list->nr; i++) {

        struct watch *watch = watch_list->items[i];
        notify_event *event = notify_event_new();

        if (iev->name[0]) {
            notify_event_set_path(event, watch->path);
            notify_event_set_filename(event, iev->name);
        } else {
            notify_event_set_path(event, dirname_s(watch->path, 1));
            notify_event_set_filename(event, basename_s(watch->path));
        }

        /*
         * Because of limitation in the information we get from inotify
         * We must do something else to find out what type a "file/link" should be.
         *
         *   if CREATE or MOVED_TO:
         *      The file is present on the filesystem, so we
         *      only need to perform a simple system call.
         *
         *   if DELETE or MOVED_FROM:
         *      The file is no longer present on the filesystem, but
         *      we will treat the symlink as a directory if we are
         *      watching the path. since we only watch directories.
         *      otherwise it may be a symlink we simple do not care about
         *      or some other file.
         */
        if (iev->mask & IN_CREATE || iev->mask & IN_MOVED_TO) {
            event->dir = (iev->mask & IN_ISDIR) ? 1 :
                is_dir(mkpath("%s/%s", event->path, event->filename));

            event->type = NOTIFY_CREATE;
        } else if (iev->mask & IN_DELETE || iev->mask & IN_MOVED_FROM) {

            if (iev->mask & IN_ISDIR) {
                event->dir = 1;
            } else {
                const char *path = mkpath("%s%s/", event->path, event->filename);

                if (inotify_map_get_wd(path) > 0) {
                    inotify_unmap_path(path);
                    event->dir = 1;
                } else {
                    event->dir = 0;
                }
            }
            event->type = NOTIFY_DELETE;
        } else if (iev->mask & IN_UNMOUNT) {
            event->dir = 1;
            event->type = NOTIFY_DELETE;
        } else {
            event->type = NOTIFY_UNKNOWN;
        }

        queue_enqueue(event_queue, event);
    }

    list_destroy(watch_list);
}

notify_event* inotify_read(void) {

    char buf[4096];
    int read, offset = 0;

    read = inotify_backend_read(buf, sizeof(buf));
    while(read > offset) {
        struct inotify_event *ev = (struct inotify_event*) &buf[offset];
        proc_event(ev);
        offset += sizeof(struct inotify_event) + ev->len;
    }
    return queue_dequeue(event_queue);
}
