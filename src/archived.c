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

#include <mysql/mysql.h>
#include <ini/iniparser.h>

#include "notify.h"
#include "xalloc.h"
#include "util.h"
#include "debug.h"

struct config {
    char *host;
    int   port;
    char *username;
    char *password;
    char *database;
    char *table;

    unsigned long thread;
    MYSQL* connection;
    my_bool reconnect;
};

static struct config db = { NULL, 3306, NULL, NULL, NULL, NULL, 0, NULL, 0 };
static dictionary *config = NULL;

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
    char *stmt = xmalloc(strlen(stmt_create) + strlen(db.table) - 1);

    sprintf(stmt, stmt_create, db.table);

    /* Run mysql query */
    ret = mysql_query(db.connection, stmt);

    /* Make sure query was successfull */
    if (ret != 0) {
        free(stmt);
        return 6;
    }

    /* Build new query */
    stmt = xrealloc(stmt, strlen(stmt_trunc) + strlen(db.table) - 1);
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
static int init_db() {

    /* Init. database connection */
    db.connection = mysql_init(NULL);

    /* Set mysql options */
    mysql_options(db.connection, MYSQL_OPT_RECONNECT, &db.reconnect);

    /* Connect to database */
    if (!mysql_real_connect(db.connection, db.host, db.username, db.password, db.database, db.port, NULL, 0)) {
        fprintf(stderr, "mysql: Could not connect to database (%s)\n", mysql_error(db.connection));
        return -1;
    }

    /* Save mysql thread id */
    db.thread = mysql_thread_id(db.connection);

    /* Setup database */
    return database_setup();
}

static int load_dbconf(const char *file) {

    int status = 0;

    if (config)
        return -1;

    if (file_exists(file)) {
        config = iniparser_load(file);
        if (NULL == config) {
            fprintf(stderr, "Could not load configuration file '%s'\n", file);
            return -1;
        }
    } else {
        fprintf(stderr, "Configuration file '%s' don't exist\n", file);
        return -1;
    }

    db.host      = iniparser_getstring(config, "mysql:host", NULL);
    db.port      = iniparser_getint(config, "mysql:port", 3306);
    db.username  = iniparser_getstring(config, "mysql:username", NULL);
    db.password  = iniparser_getstring(config, "mysql:password", NULL);
    db.database  = iniparser_getstring(config, "mysql:database", NULL);
    db.table     = iniparser_getstring(config, "mysql:table", NULL);
    db.reconnect = iniparser_getboolean(config, "mysql:reconnect", 1);

    if (NULL == db.host) {
        fprintf(stderr, "Missing 'host' in configuration\n");
        status = -1;
    }
    if (NULL == db.username) {
        fprintf(stderr, "Missing 'username' in configuration\n");
        status = -1;
    }
    if (NULL == db.password) {
        fprintf(stderr, "Missing 'password' in configuration\n");
        status = -1;
    }
    if (NULL == db.database) {
        fprintf(stderr, "Missing 'database' in configuration\n");
        status = -1;
    }
    if (NULL == db.table) {
        fprintf(stderr, "Missing 'table' in configuration\n");
        status = -1;
    }

    return status;
}

/*
 * Close database connection
 */
static int mysql_exit() {

    /* Close database connection */
    mysql_close(db.connection);

    /* memory leak fix */
    mysql_library_end();

    return 0;
}

/* Only way to exit the application properly
   when in main loop is by signal */
static void clean_exit(int excode) {

	time_t t = time(NULL);

	notify_exit();

    /* Clean mysql */
	mysql_exit();

    if (config)
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
		/* don't know why, but everything goes bananas if we keep executing */
		clean_exit(EXIT_SUCCESS);
	}
}

/*
 * Process events
 */
static int process(notify_event *event) {

    int ret = 0, dir = 0;
    char *stmt;

    dprint("%s: (%c) %s%s\n", notify_event_typetostr(event),
        event->dir ? 'D' : 'F', event->path, event->filename);

    /* Skip if event is unknown */
    if (NOTIFY_UNKNOWN == event->type)
        return 0;

    if (mysql_ping(db.connection) != 0) {
        fprintf(stderr, "mysql: Lost connection to database. Could not reconnect\n");
        return -1;
    }
    
    /* Insert new row in database */
    if (NOTIFY_CREATE == event->type) {

        char stmt_insert[] = "INSERT INTO `%s` (`Path`, `Base`, `Type`, `Status`, `Date`) VALUES('%s','%s','%i','0', NOW())";

        stmt = xmalloc(strlen(stmt_insert) + strlen(db.table) + strlen(event->path)*2 + strlen(event->filename)*2 - 6);

        /* Escape paths */
        char *escaped_path = xmalloc(strlen(event->path) * 2 + 1);
        char *escaped_filename = xmalloc(strlen(event->filename) * 2 + 1);

        mysql_real_escape_string(db.connection, escaped_path, event->path, strlen(event->path));
        mysql_real_escape_string(db.connection, escaped_filename, event->filename, strlen(event->filename));
        
        /* Create mysql query */
        sprintf(stmt, stmt_insert, db.table, escaped_path, escaped_filename, event->dir == 1);

        ret = mysql_real_query(db.connection, stmt, strlen(stmt));

        /* Clean up */
        xfree(stmt);
        xfree(escaped_path);
        xfree(escaped_filename);
        
    /* Delete row in database */
    } else if (NOTIFY_DELETE == event->type) {

        char stmt_delete[] = "DELETE FROM `%s` WHERE `Path` LIKE '%s%s%%' OR (`Path` = '%s' AND `Base` = '%s')";

        /* Escape paths */
        char *escaped_path = xmalloc(strlen(event->path) * 2 + 1);
        char *escaped_filename = xmalloc(strlen(event->filename) * 2 + 1);

        mysql_real_escape_string(db.connection, escaped_path, event->path, strlen(event->path));
        mysql_real_escape_string(db.connection, escaped_filename, event->filename, strlen(event->filename));

        stmt = xmalloc(strlen(stmt_delete) + strlen(db.table) + strlen(escaped_path)*2 + strlen(escaped_filename)*2 + 1);

        /* Create mysql query */
        sprintf(stmt, stmt_delete, db.table, escaped_path, escaped_filename, escaped_path, escaped_filename);

        ret = mysql_query(db.connection, stmt);

        /* Clean up */
        xfree(stmt);
        xfree(escaped_path);
        xfree(escaped_filename);
    }

#ifdef __DEBUG__
    /* Make sure query was successfull */
    if (ret != 0) {
        fprintf(stderr, "mysql: can't execute query\n");
        return -1;
    }

    if (db.thread != mysql_thread_id(db.connection)) {
        fprintf(stderr, "mysql: Connection was lost. Reconnected."
            "Old_Thread: %li, New_Thread: %li\n", db.thread, mysql_thread_id(db.connection));
    }
#endif

    return 0;
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

        process(event);

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
        fprintf(stderr, "Usage: archived [-c <config>] <Root Directory>\n"
        "config - path to the configuration file\n"
        "Root Directory - Path to indexroot. All subdirectories will be indexed.\n");

        return EXIT_FAILURE;
    }

    /* Load configuration */
    if (load_dbconf(configfile) < 0)
        return EXIT_FAILURE;

    ret = init_db();
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

    /* Setup signal handlers */
    signal(SIGTERM, sighandl);
    signal(SIGQUIT, sighandl);
    signal(SIGINT, sighandl);
    signal(SIGSEGV, sighandl);
    signal(SIGUSR1, sighandl);
    signal(SIGUSR2, sighandl);

    main_loop();

    return EXIT_SUCCESS;
}
