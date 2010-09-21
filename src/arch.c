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

static dictionary *config;


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

        if (indexer_pending() && !notify_is_ready()) {
#ifdef __DEBUG__
printf("sched: running indexer\n");
#endif
            indexer_run(15);
            continue;
        }

#ifdef __DEBUG__
printf("sched: notify block\n");  
#endif

        event = notify_read(-1);

        if (event == NULL)
            continue;

#ifdef __DEBUG__
printf("-- EVENT --\n"
" TYPE: %s\n"
" DIR: %i\n"
" PATH: %s%s\n"
"---------------\n", notify_event_typetostr(event), event->dir, event->path, event->filename);
#endif

        switch(event->type) {
            case NOTIFY_MOVE_TO :
                if (event->dir)
                    indexer_register(event->path, event->filename);
            case NOTIFY_CREATE :
                break;
            case NOTIFY_MOVE_FROM :
            case NOTIFY_DELETE :
                break;
        }

        status = output_process(event);
        if (0 != status) {
            fprintf(stderr,"%s",output_error(status));
        }

#ifdef __DEBUG__
notify_stat();
#endif
    }
}

int main(int argc, char **argv) {

    // Validate arguments
    if (argc != 2) {

        printf("Usage: %s <Root Directory>\n"
        "Root Directory - Path to indexroot. All subdirectories will be indexed.\n", argv[0]);

        return EXIT_FAILURE;
    }


    // Load configuration
    config = iniparser_load("config.ini");
    if (NULL == config) {
        fprintf(stderr,"Could not load configuration");
        return EXIT_FAILURE;
    }



    // Setup signal handlers
    signal(SIGTERM, sighandl);
    signal(SIGKILL, sighandl);
    signal(SIGQUIT, sighandl);
    signal(SIGINT, sighandl);
    signal(SIGSEGV, sighandl);
    signal(SIGUSR1, sighandl);
    signal(SIGUSR2, sighandl);

    // Connect to database
    int status = output_init(config);
    if (0 != status) {
        fprintf(stderr,"%s",output_error(status));
        return EXIT_FAILURE;
    }


    notify_init();

    if(notify_add_watch(argv[1]) == -1) {
            fprintf(stderr, "Invalid path: %s\n", argv[1]);
            return EXIT_FAILURE;
    }

    indexer_register(argv[1], NULL);

    arch_loop();

    return EXIT_SUCCESS;
}
