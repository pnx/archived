/*
 *   Debugging client
 * 
 *   Copyright (C) 2010  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../notify.h"

static void p_exit_time() {

    time_t t = time(NULL);
    printf("\nprocess exit at: %s", ctime(&t));
}

static void sighandler(int sig) {

    p_exit_time();

    if (sig == SIGSEGV) {
        fprintf(stderr, "Segmentation fault\n");
        abort();
    }

    notify_exit();
    exit(EXIT_SUCCESS);
}

static void pevent(notify_event *ev) {

    const char *strtype = notify_event_typetostr(ev);
    char slash = ev->dir ? '/' : '\0';

    printf("%s : %s%s%c\n", strtype, ev->path, ev->filename, slash);
}

static void mainloop() {

    notify_event *event;

    for(;;) {

        event = notify_read();

        if (event == NULL)
            continue;

        pevent(event);

        notify_event_del(event);
    }
}

int main(int argc, char **argv) {

    int rc;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <dir>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGSEGV, sighandler);

    rc = notify_init();
    if (rc < 0)
        return EXIT_FAILURE;
    
    rc = notify_add_watch(argv[1]);
    if (rc < 0) {
        fprintf(stderr, "Invalid path: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    mainloop();
    
    return EXIT_SUCCESS;
}
