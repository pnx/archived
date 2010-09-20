/* output/mysql.c - Mysql output-driver
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#include <mysql/mysql.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "output.h"
#include "../ini/iniparser.h"
#include "../common/xalloc.h"
#include "../notify/event.h"

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

static int database_setup();

/*
 * Initialize database connection and connect to database
 */
int output_init(dictionary *config) {

    // Load database information from ini config
    db.host     = iniparser_getstring(config, "mysql:host", NULL);
    db.port     = iniparser_getint(config, "mysql:port", 3306);
    db.username = iniparser_getstring(config, "mysql:username", NULL);
    db.password = iniparser_getstring(config, "mysql:password", NULL);
    db.database = iniparser_getstring(config, "mysql:database", NULL);
    db.table    = iniparser_getstring(config, "mysql:table", NULL);

    // Return exit code for missconfiguration
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

    // Enable reconnection
    if (1 == iniparser_getboolean(config, "mysql:reconnect", 1)) {
        db.reconnect = 1;
    } else {
        db.reconnect = 0;
    }

    // Init. database connection
    db.connection = mysql_init(NULL);

    // Set mysql options
    mysql_options(db.connection, MYSQL_OPT_RECONNECT, &db.reconnect);

    // Connect to database
    if (!mysql_real_connect(db.connection, db.host, db.username, db.password, db.database, db.port, NULL, 0))
        return 6;


    // Fix for mysql versions prior to 5.0.19
    mysql_options(db.connection, MYSQL_OPT_RECONNECT, &db.reconnect);


    // Save mysql thread id
    db.thread = mysql_thread_id(db.connection);

#ifdef DB_DEBUG
    fprintf(stderr, "output_init(): %li\n", db.thread);
#endif

    // Setup database
    return database_setup();

}


/*
 * Process events
 */
int output_process(notify_event *event) {

    int ret;
    char *stmt;

    // Skip if event is unknown
    if (NOTIFY_UNKNOWN == event->type)
        return 0;

    // Insert new row in database
    if (NOTIFY_CREATE == event->type) {

        char stmt_insert[] = "INSERT INTO `%s` (`Path`, `Base`, `Type`, `Status`, `Date`) VALUES('%s','%s','1','0', NOW())";

        if(mysql_ping(db.connection) != 0) {
            return 8;
	}

        stmt = xmalloc(strlen(stmt_insert) + strlen(db.table) + strlen(event->path) + strlen(event->filename) + 1);

        // Escape paths
        char *escaped_path = xmalloc(strlen(event->path) * 2 + 1);
        char *escaped_filename = xmalloc(strlen(event->filename) * 2 + 1);

        mysql_real_escape_string(db.connection, escaped_path, event->path, strlen(event->path));
        mysql_real_escape_string(db.connection, escaped_filename, event->filename, strlen(event->filename));

        // Create mysql query
	sprintf(stmt, stmt_insert, db.table, escaped_path, escaped_filename);

	// Run mysql query
	ret = mysql_query(db.connection, stmt);

	// Clean up
	free(stmt);
	free(escaped_path);
	free(escaped_filename);

    // Delete row in database
    } else if (NOTIFY_DELETE == event->type) {


        char stmt_delete[] = "DELETE FROM `%s` WHERE `Path` LIKE '%s%s%%' OR (`Path` = '%s' AND `Base` = '%s')";

        if(mysql_ping(db.connection) != 0) {
            return 8;
	}

        stmt = xmalloc(strlen(stmt_delete) + strlen(db.table) + strlen(event->path) + strlen(event->filename) + 1);

        // Escape paths
        char *escaped_path = xmalloc(strlen(event->path) * 2 + 1);
        char *escaped_filename = xmalloc(strlen(event->filename) * 2 + 1);

        mysql_real_escape_string(db.connection, escaped_path, event->path, strlen(event->path));
        mysql_real_escape_string(db.connection, escaped_filename, event->filename, strlen(event->filename));

        // Create mysql query
	sprintf(stmt, stmt_delete, db.table, escaped_path, escaped_filename, escaped_path, escaped_filename);

	// Run mysql query
	ret = mysql_query(db.connection, stmt);

	// Clean up
	free(stmt);
	free(escaped_path);
	free(escaped_filename);

    }

    // Make sure query was successfull
    if(ret != 0) {
        return 6;
    }

#ifdef DB_DEBUG

    if(db.thread != mysql_thread_id(db.connection)) {
        fprintf(stderr, "Connection was lost. Reconnected. Old_Thread: %li, New_Thread: %li\n", db.thread, mysql_thread_id(db.connection));
    }

#endif

    return 0;
}


/*
 * Close database connection
 */
int output_exit() {

    // Close database connection
    mysql_close(db.connection);

    // another leak fix
    mysql_library_end();

    return 0;
}


/*
 * Converts output error codes to string
 */
char *output_error(int error) {

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
    }

    return "Unkown error";
}


/*
 * Database setup
 */
static int database_setup() {

    int ret;

    // Sql statements
    char stmt_create[] = "CREATE TABLE IF NOT EXISTS `%s` ("
                         "`Path` varchar(512) default NULL, "
                         "`Base` varchar(512) default NULL, "
                         "`Type` tinyint(1) default NULL, "
                         "`Status` tinyint(1) default NULL, "
                         "`Date` datetime default NULL"
                         "KEY `idx_path` (`Path`(333)),"
                         "KEY `idx_base` (`Base`(333))"
                         ") ENGINE=MyISAM DEFAULT CHARSET=utf8 ";
    
    char stmt_trunc[] = "TRUNCATE TABLE `%s`";

    // Build query
    // Notice: -1 for "%s" in stmt_create and \0
    char *stmt = (char *) xmalloc(strlen(stmt_create) + strlen(db.table) - 1);
    if (stmt == NULL || sprintf(stmt, stmt_create, db.table) < 0)
        return 7;

    // Run mysql query
    ret = mysql_query(db.connection, stmt);

    // Make sure query was successfull
    if (ret != 0) {
        free(stmt);
        return 6;
    }

    // Build new query
    stmt = (char *) xrealloc(stmt, strlen(stmt_trunc) + strlen(db.table) - 1);
    if (sprintf(stmt, stmt_trunc, db.table) < 0)
        return 8;

    // Run mysql query
    ret = mysql_query(db.connection, stmt);

    // Make sure query was successfull
    if (ret != 0) {
        free(stmt);
        return 6;
    }

    
    return 0;
}