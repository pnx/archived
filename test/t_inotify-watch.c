
#include <assert.h>
#include <stdio.h>
#include "../src/inotify-watch.h"

void print_node(struct tree *n, void *data) {

    size_t c = tree_parent_count(n);
    while(c--)
        printf("│   ");

    if (n->next == NULL && n->parent)
        printf("└──");
    else
        printf("│──");
    printf(" %s\n", ((struct watch *)n)->path);
}

int main() {

    struct watch *root = inotify_watch_new(0, "/");
    struct watch *ch1 = inotify_watch_new(1, "/var/");
    struct watch *ch2 = inotify_watch_new(2, "/mnt/");

    inotify_watch_add(root, ch1);
    inotify_watch_add(root, ch2);

    inotify_watch_add(ch1, inotify_watch_new(11, "/var/a/"));
    inotify_watch_add(ch1, inotify_watch_new(12, "/var/b/"));
    inotify_watch_add(ch2, inotify_watch_new(21, "/mnt/c/"));

    inotify_watch_add((struct watch*)ch2->tree.child, inotify_watch_new(234, "/mnt/c/a/"));

    tree_traverse((struct tree*)root, print_node, NULL);

    inotify_watch_rm(ch1);

    printf("---\n");
    tree_traverse((struct tree*)root, print_node, NULL);

    inotify_watch_add(root, ch1);

    printf("---\n");
    tree_traverse((struct tree*)root, print_node, NULL);
    
    inotify_watch_destroy(root, NULL);

    return 0;
}
