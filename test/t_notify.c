
#include <stdio.h>
#include "../src/notify.h"

int main(int argc, char *argv[]) {

    notify_event *event;

    if (argc < 2)
        return 1;

    notify_init();

    if (!notify_add_watch(argv[1]))
        return 1;

    printf("begin watching on: %s\n", argv[1]);

    for (;;) {

        event = notify_read();

        if (event == NULL)
            continue;

        printf("====================\n"
               "Type: %s\n"
               "Path: %s\n"
               "Filename: %s\n"
               "Directory: %u\n"
               "====================\n"
                , notify_event_typetostr(event), event->path,
                    event->filename, event->dir);
    }

    return 0;
}
