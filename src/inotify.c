
#include <stdlib.h>
#include "log.h"
#include "path.h"
#include "queue.h"
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

    if (wd < 0)
        return -1;

    /* unmap and remove watch */
    if (inotify_unmap_path(path) == 0) {
        logmsg(LOG_DEBUG, "remove watch: %i %s", wd, path);
        
        if (inotify_backend_ignore(wd) < 0)
            return -1;
    }
    return 0;
}

static void proc_event(struct inotify_event *iev) {

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
        
        struct watch *watch = watch_list->items[i];
        notify_event *event = notify_event_new();

        notify_event_set_path(event, watch->path);
        notify_event_set_filename(event, iev->name);
        event->dir = isdir;

        switch(iev->mask) {
        case IN_CREATE :
        case IN_MOVED_TO :
            event->type = NOTIFY_CREATE;
            break;
        case IN_DELETE :
        case IN_MOVED_FROM :
            event->type = NOTIFY_DELETE;
            inotify_ignore(mkpath("%s%s/", event->path, event->filename));
            break;
        default:
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
