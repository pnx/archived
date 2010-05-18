/* fs/tree.c - Filesystem traversal
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik.hautakoski@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <dirent.h>

#include "../common/mem.h"
#include "../common/path.h"
#include "tree.h"

#if 0
  #define pdebug(...) fprintf(stderr, __VA_ARGS__)
#else
  #define pdebug(...) 0
#endif

#define isrefdir(x) (!strcmp(x->d_name, ".") || !strcmp(x->d_name, ".."))

/* REF:1 */
static int have_perm(const char *base, const char *name) {
	
	char buf[1024];
	
	sprintf(buf, "%s%s", base, name);
	
	return access(buf, R_OK | X_OK) == 0;
}

struct tree* tree_new(const char *path) {
	
	int i;
	struct tree *t = malloc(sizeof(struct tree));
	
	if (t == NULL)
		return NULL;
	
	t->path = fmt_path(path, NULL, 0);
	
	if (t->path == NULL) {
		free(t);
		return NULL;
	}
	
	t->depth = 0;
	
	t->dirs[0] = opendir(t->path);
	
	if (!t->dirs[0]) {
		free(t->path);
		free(t);
		return NULL;
	}
	
	t->ent.name = NULL;
	t->ent.base = NULL;
	t->ent.dir  = 0;
	
	return t;
}

void tree_del(struct tree *tree) {
	
	int i;
	
	for(i=0; i < tree->depth; i++)
		closedir(tree->dirs[i]);
	
	if (tree->path != NULL)
		free(tree->path);
	
	free(tree);
}

/* REF:1 */
int tree_mvup(struct tree *tree) {
	
	char *slash;
	
	if (closedir(tree->dirs[tree->depth]) == -1) {
		perror("TREE");
		return -1;
	}
	
	if (tree->depth == 0)
		return 0;
#if __DEBUG__
		printf("tree_next_ent: tree depth is: %i \n", tree->depth);
#endif
	tree->depth--;
	
	slash = tree->path + strlen(tree->path)-1;
	if (*slash == '/')
		*slash = 0;
		
	slash = strrchr(tree->path, '/');
	*(slash+1) = 0;
	
	tree->path = realloc(tree->path, sizeof(char) * (strlen(tree->path)+1));
	
	return 1;
}

/* REF:1 */
int tree_mvdown(struct tree *tree, const char *dir) {

	char buf[PATH_MAX];
	DIR  *ds;
	
	if (tree->depth >= MAX_DEPTH-1)
		return 0;

	sprintf(buf, "%s%s/\0", tree->path, dir);
	
	if ((ds = opendir(buf)) == NULL) {
		if (errno != EACCES)
			perror("DIROPEN");
		return 0;
	}
	
	tree->path = realloc(tree->path, sizeof(char) * (strlen(buf)+1));
	memcpy(tree->path, buf, strlen(buf)+1);
	
	tree->dirs[++tree->depth] = ds;
	
	return 1;
}

struct entry* tree_cpy_ent(struct entry *ent) {
	
	struct entry *ptr = malloc(sizeof(struct entry));
	
	if (ptr == NULL)
		return NULL;
	
	memcpy(ptr, ent, sizeof(struct entry));
	
	return ptr;
}

/*
 * NOTE: only read from the struct returned and DO NOT free it.
 * use tree_cpy_ent() if you need to store it.
 */
struct entry* tree_next_ent(struct tree *tree) {

	struct dirent *ent = NULL;
	
	if (tree == NULL)
		return NULL;
	
	if (tree->ent.dir)
		tree_mvdown(tree, tree->ent.name);
	
	for(;;) {
		
		ent = readdir(tree->dirs[tree->depth]);

		if (ent == NULL) {
			
			if (tree_mvup(tree))
				continue;
			
			tree_del(tree);
			return NULL;
		} else if (isrefdir(ent)) {
			continue;

		} else if (ent->d_type == 4) {

			if (have_perm(tree->path, ent->d_name))
				break;

		} else {
			break;
		}

	}

	tree->ent.dir  = ent->d_type == 4;
	tree->ent.base = tree->path;
	tree->ent.name = &ent->d_name[0];

	return &tree->ent;
}
