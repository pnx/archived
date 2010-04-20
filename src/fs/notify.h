
/*
 * Archived file-system notification
 * 
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
