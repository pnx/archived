/* fs/tree.h - Filesystem traversal
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef _FS_TREE_H

#define _FS_TREE_H

#define MAX_DEPTH 0x20

#include <dirent.h>

struct tree {
	char *path;
	DIR  *dirs[MAX_DEPTH];
	unsigned char depth;
	/* REF:2 */
	struct entry {
		char *name;
		char *base;
		unsigned char dir;
	} ent;
};

struct tree* tree_new(const char *path);

void tree_del(struct tree *tree);

int tree_mvup(struct tree *tree);

int tree_mvdown(struct tree *tree, const char *dir);

struct entry* tree_cpy_ent(struct entry *ent);

struct entry* tree_next_ent(struct tree *tree);

#endif /* _FS_TREE_H */
