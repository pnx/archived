
#include <assert.h>
#include <stdio.h>
#include "../src/fs/queue.h"

int main() {

    int i, map[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	queue_t *q = init_queue();

    for(i=0; i < 10; i++)
        queue_enqueue(q, &map[i]);

    assert(queue_num_items(q) == 10);

    for(i=0; i < 4; i++) {
        int *c = queue_dequeue(q);
        
        if (c == NULL)
            continue;
            
        assert(c == &map[i]);
    }

    for(i=4; i < 10; i++)
        queue_enqueue(q, &map[i]);

    queue_destroy(q);

	return 0;
}
