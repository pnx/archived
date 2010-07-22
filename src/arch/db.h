/* arch/db.h - database API
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __ARCH_DB_H

#define __ARCH_DB_H

int arch_db_init(char *host, char *username, char *password, char *database, char *table);

int arch_db_insert(const char *path, const char *filename, const int isdir);

int arch_db_delete(const char *path, const char *filename);

void arch_db_close();

#endif
