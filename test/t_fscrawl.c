
#include <stdio.h>
#include <time.h>
#include "../src/fscrawl.h"

int main(int argc, char *argv[]) {

    fscrawl_t crawl;
    unsigned int c_ent = 0, verbose = 0;
    time_t t1, t2, tdiff;

    if (argc < 2)
        return 1;

    if (argc > 2 && (argv[2][0] == '1' && argv[2][1] == '\0'))
        verbose = 1;

    crawl = fsc_open(argv[1]);

    if (crawl == NULL) {
        printf("Invalid path\n");
        return 1;
    }

    t1 = time(NULL);
    for(;;) {
        fs_entry *ent = fsc_read(crawl);

        if (!ent)
            break;

        if (verbose)
            printf("%s%s%c\n", ent->base, ent->name,
                ent->dir ? '/' : '\0');

        c_ent++;
    }
    t2 = time(NULL);

    fsc_close(crawl);

    tdiff = t2 - t1;
    printf("Nodes: %u\n"
           "Time (sec): %ld\n", c_ent, tdiff);

    printf("Node/sec: ");
    if (tdiff) {
        printf("%ld\n", c_ent / tdiff);
    } else {
        puts("INF");
    }

    return 0;
}
