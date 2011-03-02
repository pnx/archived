
#ifndef __INOTIFY_H
#define __INOTIFY_H

#include "event.h"

int inotify_init(void);

void inotify_exit(void);

int inotify_watch(const char *path);

int inotify_ignore(const char *path);

notify_event* inotify_read(void);

#endif /* __INOTIFY_H */
