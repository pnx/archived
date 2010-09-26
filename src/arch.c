/* arch.c
 *
 *  (C) Copyright 2010 Henrik Hautakoski <henrik@fiktivkod.org>
 *  (C) Copyright 2010 Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "output/output.h"
#include "notify/notify.h"
#include "ini/iniparser.h"
#include "common/util.h"

static dictionary *config = NULL;


/* only way to exit the application properly
   when in main loop is by signal */
static void clean_exit(int excode) {
	
	time_t t = time(NULL);
	
	notify_cleanup();

        // Clean output
	int status = output_exit();
        if (0 != status) {
            fprintf(stderr,"%s",output_error(status));
        }

        // Clean config
        iniparser_freedict(config);
	
	printf("\nprocess exit at: %s", ctime(&t));
	exit(excode);
}

/* signal handler */
static void sighandl(int sig) {
	
	switch(sig) {
	/* normal exit signals */
	case SIGTERM :
	case SIGKILL :
	case SIGQUIT :
	case SIGINT  :
		clean_exit(EXIT_SUCCESS);
	/* segmentation violation, let user now */
	case SIGSEGV :
		fprintf(stderr, "SEGFAULT: o no he didn't\n");
		clean_exit(EXIT_FAILURE);
	case SIGUSR1 :
	case SIGUSR2 :
		printf("notify stat:\n");
		notify_stat();
		/* don't know why, but everything goes bananas if we keep executing */
		clean_exit(EXIT_SUCCESS);
	}
}

void arch_loop() {

    int status;
    notify_event *event;

    for(;;) {

        event = notify_read();

        if (event == NULL)
            continue;

        status = output_process(event);
        if (status)
            fprintf(stderr,"%s",output_error(status));

        notify_event_del(event);
    }
}

int main(int argc, char **argv) {

    int status;

    /* Validate arguments */
    if (argc != 2) {

        printf("Usage: %s <Root Directory>\n"
        "Root Directory - Path to indexroot. All subdirectories will be indexed.\n", argv[0]);

        return EXIT_FAILURE;
    }

    /* Load configuration */
    if (file_exists("config.ini")) {
        config = iniparser_load("config.ini");
        if (NULL == config) {
            fprintf(stderr, "Could not load configuration");
            return EXIT_FAILURE;
        }
    }
    
    /* Setup signal handlers */
    signal(SIGTERM, sighandl);
    signal(SIGKILL, sighandl);
    signal(SIGQUIT, sighandl);
    signal(SIGINT, sighandl);
    signal(SIGSEGV, sighandl);
    signal(SIGUSR1, sighandl);
    signal(SIGUSR2, sighandl);

    status = output_init(config);
    if (status) {
        fprintf(stderr, "%s", output_error(status));
        return EXIT_FAILURE;
    }

    status = notify_init();
    if (status == -1)
        return EXIT_FAILURE;

    status = notify_add_watch(argv[1]);
    if (status == -1) {
        fprintf(stderr, "Invalid path: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    arch_loop();

    return EXIT_SUCCESS;
}
