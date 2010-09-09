/* notify/queue.c
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

#include <assert.h>
#include <stdint.h>
#include <malloc.h>
#include "queue.h"

#define BLOCK_SIZE (1<<7)

#define init(q) \
    do { \
        q->tail.n = q->head.n = NULL; \
        q->tail.i = q->head.i = 0; \
    } while(0)
    
/* linked list node holding a chunk of queue elements */
struct node {
    void *block[BLOCK_SIZE];
	struct node *next;
};

struct ref {
    uint16_t i;
    struct node *n;
};

struct __queue {
	struct ref tail;
	struct ref head;
};

static void alloc_node(struct ref *head) {

	struct node *n = malloc(sizeof(struct node));
    assert(n != NULL);

    n->next = NULL;
	head->n = head->n->next = n;
	head->i = 0;
}

static void dealloc_node(struct ref *tail) {

	struct node *next = tail->n->next;
    assert(next != NULL);

    free(tail->n);
	
	tail->n = next;
	tail->i = 0;
}

queue_t init_queue() {

    queue_t q = malloc(sizeof(struct __queue));

    init(q);

    return q;
}

void queue_clear(queue_t q) {

    struct node *t, *n;

    if (q == NULL)
        return;

    n = q->tail.n;

    while(n) {
        t = n->next;
        free(n);
        n = t;
    }

    init(q);
}

void queue_destroy(queue_t q) {

    queue_clear(q);
    free(q);
}

void queue_enqueue(queue_t q, void *obj) {

	if (q->head.n == NULL) {
		q->tail.n = q->head.n = malloc(sizeof(struct node));
        assert(q->tail.n != NULL);
	} else if (q->head.i + 1 >= BLOCK_SIZE) {
		alloc_node(&q->head);
    } else {
        q->head.i++;
    }

	q->head.n->block[q->head.i] = obj;
}

void* queue_dequeue(queue_t q) {

    void *obj;

    if (queue_isempty(q))
        return NULL;

    obj = q->tail.n->block[q->tail.i++];

    if (q->tail.n == q->head.n && q->tail.i > q->head.i) {
        free(q->head.n);
        init(q);
    } else if (q->tail.i >= BLOCK_SIZE) {
        dealloc_node(&q->tail);
    }
    
    return obj;
}

int queue_isempty(queue_t q) {

    if (q == NULL)
        return -1;

    return q->head.n == NULL && q->head.n == q->tail.n;
}

size_t queue_num_items(queue_t q) {

    if (queue_isempty(q))
        return 0;
    if (q->head.n == q->tail.n)
        return (q->head.i + 1) - q->tail.i;

    size_t len = (BLOCK_SIZE - q->tail.i) + (q->head.i + 1);
    struct node *n = q->tail.n->next;

    for(; n != q->head.n; n = n->next)
        len += BLOCK_SIZE;

    return len;
}

