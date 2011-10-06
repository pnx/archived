/* database/mysql.c - mysql implementation
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */
#include <mysql/mysql.h>
#include <ini/iniparser.h>
#include "../database.h"
#include "../xalloc.h"

static struct {
    char *host;
    int   port;
    char *username;
    char *password;
    char *database;
    char *table;

    unsigned long thread;
    MYSQL* connection;
    my_bool reconnect;
} db = { NULL, 3306, NULL, NULL, NULL, NULL, 0, NULL, 0 };

/* Helper for escaping strings in mysql queries. */
static char* escape(const char *str) {

    size_t len = strlen(str);
    char *esc  = xmalloc(len * 2 + 1);

    mysql_real_escape_string(db.connection, esc, str, len);

    return esc;
}

static int load_dbconf(dictionary *conf) {

    int status = 0;

    db.host      = iniparser_getstring(conf, "mysql:host", NULL);
    db.port      = iniparser_getint(conf, "mysql:port", 3306);
    db.username  = iniparser_getstring(conf, "mysql:username", NULL);
    db.password  = iniparser_getstring(conf, "mysql:password", NULL);
    db.database  = iniparser_getstring(conf, "mysql:database", NULL);
    db.table     = iniparser_getstring(conf, "mysql:table", NULL);
    db.reconnect = iniparser_getboolean(conf, "mysql:reconnect", 1);

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
int database_init(dictionary *conf) {

    if (load_dbconf(conf) < 0)
        return -1;

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

int database_insert(const char *path, const char *filename, const int isdir) {

    int ret;
    char stmt_insert[] = "INSERT INTO `%s` (`Path`, `Base`, `Type`, `Status`, `Date`) VALUES('%s','%s','%i','0', NOW())";
    char *stmt, *escaped_path, *escaped_filename;

    if (mysql_ping(db.connection) != 0) {
        fprintf(stderr, "mysql: Lost connection to database. Could not reconnect\n");
        return -1;
    }

    /* Escape the strings */
    escaped_path = escape(path);
    escaped_filename = escape(filename);

    stmt = xmalloc(strlen(stmt_insert) + strlen(db.table) + strlen(escaped_path) + strlen(escaped_filename) - 6);

    /* Create mysql query */
    sprintf(stmt, stmt_insert, db.table, escaped_path, escaped_filename, isdir != 0);

    ret = mysql_query(db.connection, stmt);

    xfree(stmt);
    xfree(escaped_path);
    xfree(escaped_filename);

    if (ret != 0) {
        fprintf(stderr, "mysql: %s\n", mysql_error(db.connection));
        return -1;
    }
    return 0;
}

int database_delete(const char *path, const char *filename) {

    int ret;
    char stmt_delete[] = "DELETE FROM `%s` WHERE `Path` LIKE '%s%s%%' OR (`Path` = '%s' AND `Base` = '%s')";
    char *stmt;

    /* Escape paths */
    char *escaped_path = escape(path);
    char *escaped_filename = escape(filename);

    stmt = xmalloc(strlen(stmt_delete) + strlen(db.table) + strlen(escaped_path)*2 + strlen(escaped_filename)*2 + 1);

    /* Create mysql query */
    sprintf(stmt, stmt_delete, db.table, escaped_path, escaped_filename, escaped_path, escaped_filename);

    ret = mysql_query(db.connection, stmt);

    xfree(stmt);
    xfree(escaped_path);
    xfree(escaped_filename);

    if (ret != 0) {
        fprintf(stderr, "mysql: %s\n", mysql_error(db.connection));
        return -1;
    }
    return 0;
}

int database_close() {

    mysql_close(db.connection);

    /* memory leak fix */
    mysql_library_end();

    return 0;
}
