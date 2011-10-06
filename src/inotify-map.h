/* inotify-map.h
 *
 *   Copyright (C) 2011 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __INOTIFY_MAP_H
#define __INOTIFY_MAP_H

#include "list.h"
#include "inotify-watch.h"

void inotify_map(int wd, const char *path);

int inotify_unmap_wd(int wd);

int inotify_unmap_path(const char *path);

void inotify_unmap_all();

int inotify_map_get_wd(const char *path);

struct list* inotify_map_get_path(int wd);

int inotify_map_isempty();

#endif /* __INOTIFY_MAP_H */
