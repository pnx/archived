/* fs/notify_event.h - event data structure and operation's for notify API
 * 
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef _NOTIFY_EVENT_H

#define _NOTIFY_EVENT_H

#include <stdint.h>

/* event types */
#define NOTIFY_UNKNOWN 	   (1 << 0)
#define NOTIFY_CREATE 	   (1 << 1)
#define NOTIFY_DELETE 	   (1 << 2)
#define NOTIFY_MOVE_FROM   (1 << 3)
#define NOTIFY_MOVE_TO     (1 << 4)

typedef struct {
	uint8_t type; 	/* type of event */
	uint8_t dir; 	/* non zero if event is triggered on a directory */
	char *path; 	/* path of the triggered event */
	char *filename; /* the filename event was triggered on */
} notify_event;

notify_event* notify_event_new();

void notify_event_del(notify_event *event);

void notify_event_clear(notify_event *event);

void notify_event_set_path(notify_event *event, const char *path);

void notify_event_set_filename(notify_event *event, const char *filename);

void notify_event_set_dir(notify_event *event, uint8_t dir);

void notify_event_set_type(notify_event *event, uint8_t type);

const char* notify_event_typetostr(notify_event *event);

#endif /* _NOTIFY_EVENT_H */
