/* queue.c
 *
 *   Copyright (C) 2010       Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "xalloc.h"
#include "queue.h"

#define BLOCK_SIZE (1<<7)

/* linked list node holding a chunk of queue elements */
struct node {
    void *block[BLOCK_SIZE];
    struct node *next;
};

struct ref {
    unsigned int i;
    struct node *n;
};

struct __queue {
    struct ref tail;
    struct ref head;
};

static void alloc_node(struct ref *head) {

    struct node *n = xmalloc(sizeof(struct node));

    n->next = NULL;
    head->n = head->n->next = n;
    head->i = 0;
}

static void dealloc_node(struct ref *tail) {

    struct node *next = tail->n->next;

    if (next) {
        xfree(tail->n);
        tail->n = next;
        tail->i = 0;
    }
}

queue_t queue_init() {

    queue_t q = xmalloc(sizeof(struct __queue));
    q->tail.n = q->head.n = xmalloc(sizeof(struct node));
    q->tail.n->next = q->head.n->next = NULL;
    q->tail.i = q->head.i = 0;

    return q;
}

void queue_destroy(queue_t q) {

    if (q) {
        struct node *n = q->tail.n;
        while(n) {
            struct node *next = n->next;
            xfree(n);
            n = next;
        }
        xfree(q);
    }
}

void queue_enqueue(queue_t q, void *obj) {

    if (q == NULL)
        return;

    if (q->head.i >= BLOCK_SIZE) {
          alloc_node(&q->head);
    }

    q->head.n->block[q->head.i++] = obj;
}

void* queue_dequeue(queue_t q) {

    void *obj;

    if (queue_isempty(q))
        return NULL;

    obj = q->tail.n->block[q->tail.i++];

    if (q->tail.n == q->head.n && q->tail.i >= q->head.i) {
        q->tail.i = q->head.i = 0;
    } else if (q->tail.i >= BLOCK_SIZE) {
        dealloc_node(&q->tail);
    }

    return obj;
}

int queue_isempty(queue_t q) {

    if (q == NULL)
        return -1;

    return q->head.n == q->tail.n
        && q->head.i == 0 && q->tail.i == 0;
}

size_t queue_num_items(queue_t q) {

    size_t len;
    struct node *n;

    if (queue_isempty(q))
        return 0;
    if (q->head.n == q->tail.n)
        return q->head.i - q->tail.i;

    len = (BLOCK_SIZE - q->tail.i) + q->head.i;
    for(n = q->tail.n->next; n != q->head.n; n = n->next)
        len += BLOCK_SIZE;
    return len;
}
