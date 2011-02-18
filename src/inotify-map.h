/* inotify-map.h
 * 
 *  (C) Copyright 2011 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef __INOTIFY_MAP_H
#define __INOTIFY_MAP_H

void inotify_map(int wd, const char *path);

int inotify_unmap_wd(int wd);

int inotify_unmap_path(const char *path);

void inotify_unmap_all();

int inotify_map_get_wd(const char *path);

char** inotify_map_get_path(int wd);

char** inotify_map_lookup_by_path(const char *path);

int inotify_map_isempty();

#endif /* __INOTIFY_MAP_H */
