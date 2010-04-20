
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
