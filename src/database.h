/* database.h - database driver API
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __DATABASE_H
#define __DATABASE_H

#include <ini/dictionary.h>

int database_init(dictionary *conf);

int database_insert(const char *path, const char *filename, const int isdir);

int database_delete(const char *path, const char *filename);

int database_close();

#endif /* __DATABASE_H */
