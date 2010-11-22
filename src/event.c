/* event.c
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "event.h"

#define dealloc_data(ev) \
	if (ev->path) \
		free(ev->path); \
	if (ev->filename) \
		free(ev->filename)

static void init_event(notify_event* ev) {
	
	ev->filename = NULL;
	ev->path 	 = NULL;
	ev->type 	 = NOTIFY_UNKNOWN;
	ev->dir 	 = 0;
}

/*
 * Create event
 */
notify_event* notify_event_new() {
	
	notify_event *ev = malloc(sizeof(notify_event));
	
	if (ev == NULL)
		return NULL;
		
	init_event(ev);
	
	return ev;
}

/*
 * Destroy event
 */
void notify_event_del(notify_event *event) {

	if (event == NULL)
		return;
		
	dealloc_data(event);
	free(event);
}

/*
 * dealloc memory and set default values
 */
void notify_event_clear(notify_event *event) {

	if (event == NULL)
		return;

	dealloc_data(event);
	init_event(event);
}

/*
 * Set event path
 */
void notify_event_set_path(notify_event *event, const char *path) {
	
	char *ptr;
	
	if (event == NULL || path == NULL)
		return;
	
	ptr = realloc(event->path, sizeof(char) * (strlen(path)+1));
	
	if (ptr == NULL)
		return;
		
	event->path = ptr;
	
	memcpy(event->path, path, strlen(path)+1);
}

/*
 * Set event filename
 */
void notify_event_set_filename(notify_event *event, const char *filename) {
	
	char *tmp;
	
	if (event == NULL || filename == NULL)
		return;
	
	
	tmp = realloc(event->filename, sizeof(char) * (strlen(filename)+1));
	
	if (tmp == NULL)
		return;
	
	memcpy(tmp, filename, strlen(filename)+1);
	
	event->filename = tmp;
}

/* set directory */
void notify_event_set_dir(notify_event *event, uint8_t dir) {

	if (event == NULL)
		return;

	event->dir = dir;
}

void notify_event_set_type(notify_event *event, uint8_t type) {
	
	if (event == NULL)
		return;
		
	event->type = type;
}

const char* notify_event_typetostr(notify_event *event) {

    if (!event)
        return "(null)";
    
	switch(event->type) {
		case NOTIFY_CREATE :
			return "CREATE";
		case NOTIFY_DELETE :
			return "DELETE";
			break;
		case NOTIFY_MOVE_FROM :
			return "MOVE_FROM";
			break;
		case NOTIFY_MOVE_TO :
			return "MOVE_TO";
			break;
		default:
			return "UNKNOWN";
	}
}
