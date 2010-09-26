/* output/stdout.c - Standard output output-driver
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "output.h"
#include "../notify/event.h"

/*
 * Init. output-driver. Stdout has nothing
 * to setup.
 */
int output_init(dictionary *config) {
    printf("Running output_init()\n");
    return 0;
}

int output_process(notify_event *event) {

    char *event_string;

    switch (event->type) {
        case NOTIFY_CREATE:
            event_string = "create";
            break;
        case NOTIFY_DELETE:
            event_string = "delete";
            break;
        case NOTIFY_MOVE_FROM:
            event_string = "move_from";
            break;
        case NOTIFY_MOVE_TO:
            event_string = "move_to";
            break;
        default:
            event_string = "unknown";
    }

    printf("output_process: {Path: %s, Filename: %s, Type: %s}\n", event->path, event->filename, event_string);
    return 0;
}

char *output_error(int error) {
    return "Some kind of error";
}

int output_exit(void) {
    printf("Running output_exit()\n");
    return 0;
}
