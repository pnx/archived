/* indexer.c
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#include <malloc.h>
#include <string.h>
#include "common/debug.h"
#include "common/path.h"
#include "output/output.h"
#include "notify/notify.h"
#include "notify/tree.h"
#include "indexer.h"

static char *stack[256];
static unsigned char st_size = 0;
static struct tree *current = NULL;

/* REF:1 */
static void index_entry(struct entry *ent) {

    static char buf[1024];
#if __DEBUG__
		printf("DEBUG: index_entry adding to db: %s %s %d\n", ent->base, ent->name, ent->dir);
#endif

    
    // Event change, just for test.
    notify_event ev;

    ev.path = ent->base;
    ev.filename = ent->name;
    ev.type = NOTIFY_CREATE;
    int status = output_process(&ev);
    if (0 != status)
        fprintf(stderr,"%s",output_error(status));

    if (ent->dir) {
        memcpy(buf, ent->base, strlen(ent->base));
        memcpy(buf + strlen(ent->base), ent->name, strlen(ent->name)+1);
        notify_add_watch(buf);
    }
    
#ifdef __DEBUG__
    printf("Index: %s%s\n", ent->base, ent->name);
#endif
}

static void add(const char *path) {
	
	char *newstr;

	if (st_size >= 255)
		return;
	
	newstr = malloc(sizeof(char) * (strlen(path)+1));
	
	if (newstr == NULL)
		return;
	
	memcpy(newstr, path, strlen(path)+1);
	
	stack[st_size] = newstr;
	
	st_size++;
}

/* we only shift the stack when getting the 
 * next tree, so this will do for now. */
static int rm() {
	
	int i;
	
	current = NULL;
	
	while(current == NULL) {
		
		if (st_size == 0)
			return 0;
	
		current = tree_new(stack[0]);
		
		free(stack[0]);
	
		st_size--;
	
		for(i=0; i < st_size; i++)
			stack[i] = stack[i+1];
	}
	
	return 1;
}

void indexer_register(const char *base, const char *name) {
	
    char *fullpath = path_normalize(base, name, 1);
    
	if (!indexer_pending()) {
#if __DEBUG__
		printf("sched: adding current index: %s\n", fullpath);
#endif
		current = tree_new(fullpath);
		free(fullpath);
		return;
	}
	
#if __DEBUG__
		printf("sched: scheduling index: %s\n", fullpath);
#endif
	
	add(fullpath);
}

void indexer_run(unsigned int num) {
	
	struct entry *ent;
	
	int i;
	
	for(i=0; i < num; i++) {
		
		ent = tree_next_ent(current);
		
		if (ent == NULL) {
			
			if (rm())
				continue;
			
			return;
		}
		
        index_entry(ent);
#if __DEBUG__
		printf("DEBUG: indexer_run(): ");
#endif
	}
}

inline int indexer_pending() {
	return current != NULL || st_size;
}
