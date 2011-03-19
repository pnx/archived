/* notify.h - filesystem notification API
 *
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef __NOTIFY_NOTIFY_H
#define __NOTIFY_NOTIFY_H

/* notify event def's and operations */
#include "event.h"

int notify_init();

void notify_exit();

int notify_add_watch(const char *path);

int notify_rm_watch(const char *path);

notify_event* notify_read();

void notify_stat();

#endif /* __NOTIFY_NOTIFY_H */
