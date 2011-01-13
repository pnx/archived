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
#include <string.h>

#include "event.h"
#include "xalloc.h"

#define dealloc_data(ev)    \
	if (ev->path)           \
		xfree(ev->path);    \
	if (ev->filename)       \
		xfree(ev->filename)

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
	
	notify_event *ev = xmalloc(sizeof(notify_event));
	
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
	xfree(event);
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
	
	if (event == NULL || path == NULL)
		return;
	
	event->path = xrealloc(event->path, strlen(path)+1);
	
	memcpy(event->path, path, strlen(path)+1);
}

/*
 * Set event filename
 */
void notify_event_set_filename(notify_event *event, const char *filename) {
	
	if (event == NULL || filename == NULL)
		return;
	
	event->filename = xrealloc(event->filename, strlen(filename)+1);
	
	memcpy(event->filename, filename, strlen(filename)+1);
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
