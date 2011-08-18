/* backend-api.h - backend driver API
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __BACKEND_API_H
#define __BACKEND_API_H

#include <ini/dictionary.h>

int backend_init(dictionary *conf);

int backend_insert(const char *path, const char *filename, const int isdir);

int backend_delete(const char *path, const char *filename);

int backend_exit();

#endif /* __BACKEND_API_H */
