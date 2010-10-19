/* queue.h
 *
 *   Copyright (C) 2010       Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NOTIFY_QUEUE_H
#define __NOTIFY_QUEUE_H

#include <stddef.h>

typedef struct __queue* queue_t;

queue_t queue_init(void);

void queue_enqueue(queue_t q, void *obj);

void* queue_dequeue(queue_t q);

int queue_isempty(queue_t q);

size_t queue_num_items(queue_t q);

void queue_clear(queue_t q);

void queue_destroy(queue_t q);

#endif /* __NOTIFY_QUEUE_H */
