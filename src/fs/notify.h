/* fs/notify.h - filesystem notification API
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik.hautakoski@gmail.com>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@unknown.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef _FS_NOTIFY_H

#define _FS_NOTIFY_H

/* notify event def's and operations */
#include "notify_event.h"

int notify_init();

void notify_cleanup();

int notify_add_watch(const char *path);

void notify_rm_watch(unsigned int wd);

void notify_rm_watch_path(const char *path);

void notify_print_stat();

notify_event* notify_read();

void notify_stat();

int notify_is_ready();

#endif /* _FS_NOTIFY_H */
