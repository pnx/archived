/* client/mysql.c
 *
 *  (C) Copyright 2010 Fredric Nilsson <fredric@fiktivkod.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <mysql/mysql.h>

#include "../notify/notify.h"
#include "../notify/event.h"
#include "../common/xalloc.h"
#include "../ini/iniparser.h"
#include "../common/util.h"
#include "../common/debug.h"

typedef struct {
    char *host;
    int   port;
    char *username;
    char *password;
    char *database;
    char *table;

    unsigned long thread;
    MYSQL* connection;
    my_bool reconnect;

} database;

static database db;

static dictionary *config = NULL;

/*
 * Converts error codes to string
 */
static char *client_error(int error) {

    switch (error) {
        case 1:
            return "Missing 'host' in configuration";
        case 2:
            return "Missing 'username' in configuration";
        case 3:
            return "Missing 'password' in configuration";
        case 4:
            return "Missing 'database' in configuration";
        case 5:
            return "Missing 'table' in configuration";
        case 6:
            return mysql_error(db.connection);
        case 7:
            return "Error while creating table";
        case 8:
            return "Lost connection to database. Could not reconnect";
        case 9:
            return "Missing configuration";
    }

    return "Unkown error";
}


/*
 * Close database connection
 */
static int client_exit() {

    /* Close database connection */
    mysql_close(db.connection);

    /* Another leak fix */
    mysql_library_end();

    return 0;
}


/* Only way to exit the application properly
   when in main loop is by signal */
static void clean_exit(int excode) {

	time_t t = time(NULL);

	notify_exit();

        /* Clean mysql */
	int status = client_exit();
        if (0 != status) {
            char *str = client_error(status);
            fprintf(stderr,"%s", str);
        }

        /* Clean config */
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

/*
 * Database setup
 */
static int database_setup() {

    int ret;

    /* Sql statements */
    char stmt_create[] = "CREATE TABLE IF NOT EXISTS `%s` ("
                         "`Path` varchar(512) default NULL, "
                         "`Base` varchar(512) default NULL, "
                         "`Type` tinyint(1) default NULL, "
                         "`Status` tinyint(1) default NULL, "
                         "`Date` datetime default NULL, "
                         "KEY `idx_path` (`Path`(333)), "
                         "KEY `idx_base` (`Base`(333)) "
                         ") ENGINE=MyISAM DEFAULT CHARSET=utf8 ";

    char stmt_trunc[] = "TRUNCATE TABLE `%s`";

    /* Build query
       Notice: -1 for "%s" in stmt_create and \0 */
    char *stmt = (char *) xmalloc(strlen(stmt_create) + strlen(db.table) - 1);
    if (stmt == NULL || sprintf(stmt, stmt_create, db.table) < 0)
        return 7;

    /* Run mysql query */
    ret = mysql_query(db.connection, stmt);

    /* Make sure query was successfull */
    if (ret != 0) {
        free(stmt);
        return 6;
    }

    /* Build new query */
    stmt = (char *) xrealloc(stmt, strlen(stmt_trunc) + strlen(db.table) - 1);
    if (sprintf(stmt, stmt_trunc, db.table) < 0)
        return 8;

    /* Run mysql query */
    ret = mysql_query(db.connection, stmt);

    /* Make sure query was successfull */
    if (ret != 0) {
        free(stmt);
        return 6;
    }


    return 0;
}


/*
 * Initialize database connection and connect to database
 */
static int client_init() {

    /* Load database information from ini config */
    db.host     = iniparser_getstring(config, "mysql:host", NULL);
    db.port     = iniparser_getint(config, "mysql:port", 3306);
    db.username = iniparser_getstring(config, "mysql:username", NULL);
    db.password = iniparser_getstring(config, "mysql:password", NULL);
    db.database = iniparser_getstring(config, "mysql:database", NULL);
    db.table    = iniparser_getstring(config, "mysql:table", NULL);

    /* Return exit code for missconfiguration */
    if (NULL == db.host)
        return 1;

    if (NULL == db.username)
        return 2;

    if (NULL == db.password)
        return 3;

    if (NULL == db.database)
        return 4;

    if (NULL == db.table)
        return 5;

    /* Enable reconnection */
    if (1 == iniparser_getboolean(config, "mysql:reconnect", 1)) {
        db.reconnect = 1;
    } else {
        db.reconnect = 0;
    }

    /* Init. database connection */
    db.connection = mysql_init(NULL);

    /* Set mysql options */
    mysql_options(db.connection, MYSQL_OPT_RECONNECT, &db.reconnect);

    /* Connect to database */
    if (!mysql_real_connect(db.connection, db.host, db.username, db.password, db.database, db.port, NULL, 0))
        return 6;


    /* Fix for mysql versions prior to 5.0.19 */
    mysql_options(db.connection, MYSQL_OPT_RECONNECT, &db.reconnect);


    /* Save mysql thread id */
    db.thread = mysql_thread_id(db.connection);

#ifdef DB_DEBUG
    fprintf(stderr, "output_init(): %li\n", db.thread);
#endif

    /* Setup database */
    return database_setup();

}


/*
 * Process events
 */
static int client_process(notify_event *event) {

    int ret = 0, dir = 0;
    char *stmt;

    /* Skip if event is unknown */
    if (NOTIFY_UNKNOWN == event->type)
        return 0;
    
    /* Insert new row in database */
    if (NOTIFY_CREATE == event->type) {

        char stmt_insert[] = "INSERT INTO `%s` (`Path`, `Base`, `Type`, `Status`, `Date`) VALUES('%s','%s','%i','0', NOW())";

        if(mysql_ping(db.connection) != 0) {
            return 8;
	}

        stmt = (char *)xmalloc(sizeof(char) * (strlen(stmt_insert) + strlen(db.table) + strlen(event->path)*2 + strlen(event->filename)*2 - 6));

        /* Escape paths */
        char *escaped_path = xmalloc(strlen(event->path) * 2 + 1);
        char *escaped_filename = xmalloc(strlen(event->filename) * 2 + 1);

        mysql_real_escape_string(db.connection, escaped_path, event->path, strlen(event->path));
        mysql_real_escape_string(db.connection, escaped_filename, event->filename, strlen(event->filename));

        /* dir :D */
        if (event->dir == 1) {
            dir = 1;
        }

        /* Create mysql query */
	sprintf(stmt, stmt_insert, db.table, escaped_path, escaped_filename, dir);

	/* Run mysql query */
	ret = mysql_real_query(db.connection, stmt, strlen(stmt));

	/* Clean up */
	xfree(stmt);
	xfree(escaped_path);
	xfree(escaped_filename);

    /* Delete row in database */
    } else if (NOTIFY_DELETE == event->type) {

        char stmt_delete[] = "DELETE FROM `%s` WHERE `Path` LIKE '%s%s%%' OR (`Path` = '%s' AND `Base` = '%s')";

        if(mysql_ping(db.connection) != 0) {
            return 8;
	}

        /* Escape paths */
        char *escaped_path = xmalloc(strlen(event->path) * 2 + 1);
        char *escaped_filename = xmalloc(strlen(event->filename) * 2 + 1);

        mysql_real_escape_string(db.connection, escaped_path, event->path, strlen(event->path));
        mysql_real_escape_string(db.connection, escaped_filename, event->filename, strlen(event->filename));

        stmt = xmalloc(strlen(stmt_delete) + strlen(db.table) + strlen(escaped_path)*2 + strlen(escaped_filename)*2 + 1);

        /* Create mysql query */
	sprintf(stmt, stmt_delete, db.table, escaped_path, escaped_filename, escaped_path, escaped_filename);

	/* Run mysql query */
	ret = mysql_query(db.connection, stmt);

	/* Clean up */
	xfree(stmt);
	xfree(escaped_path);
	xfree(escaped_filename);

    }

    /* Make sure query was successfull */
    if(ret != 0) {
        return 6;
    }

#ifdef __DEBUG__

    if(db.thread != mysql_thread_id(db.connection)) {
        fprintf(stderr, "Connection was lost. Reconnected. Old_Thread: %li, New_Thread: %li\n", db.thread, mysql_thread_id(db.connection));
    }

#endif

    return 0;
}


/*
 * The main loop - read events from notify API
 */
static void main_loop() {

    int status;
    notify_event *event;

    for(;;) {

        event = notify_read();

        if (event == NULL)
            continue;

        status = client_process(event);
        if (status)
            fprintf(stderr,"%s", client_error(status));

        notify_event_del(event);
    }
}

int main(int argc, char** argv) {

    /* Return value */
    int ret;

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

    ret = client_init();
    if (ret) {
        fprintf(stderr, "%s", client_error(ret));
        return EXIT_FAILURE;
    }

    ret = notify_init();
    if (ret == -1)
        return EXIT_FAILURE;

    ret = notify_add_watch(argv[1]);
    if (ret == -1) {
        fprintf(stderr, "Invalid path: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    main_loop();

    return (EXIT_SUCCESS);
}
