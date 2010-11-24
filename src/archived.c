/* archived.c
 *
 *  Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <ini/iniparser.h>

#include "notify.h"
#include "database.h"
#include "log.h"
#include "path.h"

static const char *archived_usage_str =
    "Usage: archived [-c <config>] <Root Directory>\n"
    "       config - path to the configuration file\n"
    "       Root Directory - Path to indexroot. All subdirectories will be indexed.\n";

static dictionary *config = NULL;

static int load_config(const char *file) {

    iniparser_freedict(config);

    if (is_file(file)) {
        config = iniparser_load(file);
        if (NULL == config)
            return -1;
    } else {
        fprintf(stderr, "Configuration file '%s' don't exist\n", file);
        return -1;
    }
    return 0;
}

/* Only way to exit the application properly
   when in main loop is by signal */
static void clean_exit(int excode) {

	time_t t = time(NULL);

	notify_exit();

	database_close();

    iniparser_freedict(config);

	printf("\nprocess exit at: %s", ctime(&t));
	exit(excode);
}

/* Signal handler */
static void sighandl(int sig) {

	switch(sig) {
	/* normal exit signals */
	case SIGTERM :
	case SIGKILL :
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
	}
}

/*
 * The main loop - read events from notify API
 */
static void main_loop() {

    notify_event *event;

    for(;;) {

        event = notify_read();

        if (event == NULL)
            continue;
            
        logmsg(LOG_DEBUG, "%s: (%c) %s%s", notify_event_typetostr(event),
            event->dir ? 'D' : 'F', event->path, event->filename);

        if (event->type == NOTIFY_CREATE)
            database_insert(event->path, event->filename, event->dir);
        else if (event->type == NOTIFY_DELETE)
            database_delete(event->path, event->filename);

        notify_event_del(event);
    }
}

int main(int argc, char **argv) {

    /* Return value */
    int ret;
    char *configfile = "config.ini", *rootdir;

    /* Validate arguments */
    if (argc > 3 && !strcmp(argv[1], "-c")) {
        configfile = argv[2];
        rootdir = argv[3];
    } else if (argc > 1 && strcmp(argv[1], "-c")) {
        rootdir = argv[1];
    } else {
        fputs(archived_usage_str, stderr);
        return EXIT_FAILURE;
    }

    /* Setup signal handlers */
    signal(SIGTERM, sighandl);
    signal(SIGQUIT, sighandl);
    signal(SIGINT, sighandl);
    signal(SIGSEGV, sighandl);
    signal(SIGUSR1, sighandl);
    signal(SIGUSR2, sighandl);

    if (load_config(configfile) < 0)
        return EXIT_FAILURE;

#ifdef __DEBUG__
    init_log(LOG_ALL, NULL);
#else
    if (iniparser_getboolean(config, "general:uselog", 0)) {
        char *path = iniparser_getstring(config, "general:logdir", NULL);
        init_log(LOG_INFO | LOG_WARN | LOG_CRIT, path);
    }
#endif
    
    ret = database_init(config);
    if (ret == -1)
        return EXIT_FAILURE;

    ret = notify_init();
    if (ret == -1)
        return EXIT_FAILURE;

    ret = notify_add_watch(rootdir);
    if (ret == -1) {
        fprintf(stderr, "Invalid path: %s\n", rootdir);
        return EXIT_FAILURE;
    }

    main_loop();

    return EXIT_SUCCESS;
}
