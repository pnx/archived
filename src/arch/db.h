
/*
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

#ifndef _ARCH_DB_H

#define _ARCH_DB_H

int arch_db_init(char *host, char *username, char *password, char *database, char *table);

int arch_db_insert(const char *path, const char *filename, const int isdir);

int arch_db_delete(const char *path, const char *filename);

int arch_db_truncate();

void arch_db_close();

#endif /* _DB_DATABASE_H */
