/* arch/mysql.c - Mysql implementation
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

#include "db.h"

#define STRSZ(s) (sizeof(char) * strlen(s))

static MYSQL* dbconn;
static unsigned long dbthread_id;
static char *dbtable = NULL;

int db_setup();

/*
 * Initialize database connection and connect to database
 */
int arch_db_init(char *host, char *username, char *password, char *database, char *table) {

	my_bool reconnect = 1;
	
	/* Keep tablename for querys */
	dbtable = malloc(sizeof(table));
	strcpy(dbtable,table);
	
	/* Init. database */
	dbconn = mysql_init(NULL);
	
	/* Set mysql options */
	mysql_options(dbconn, MYSQL_OPT_RECONNECT, &reconnect);
		
	/* Connect to database */
	if (!mysql_real_connect(dbconn, host, username, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(dbconn));
		return 0;
	}
	
	/* Fix for mysql versions prior to 5.0.19 */
	mysql_options(dbconn, MYSQL_OPT_RECONNECT, &reconnect);
	
	
	/* Save mysql thread id */
	dbthread_id = mysql_thread_id(dbconn);
	
#ifdef DB_DEBUG
	fprintf(stderr, "Mysql init: %li\n", dbthread_id);
#endif

    /* setup database */
    db_setup();

	return 1;
}

/*
 * Close database connection
 */
void arch_db_close() {
	
	/* dbtable never free'd */
	if (dbtable != NULL) {
		free(dbtable);
		dbtable = NULL;
	}
	
	mysql_close(dbconn);
	
	/* another leak fix */
	mysql_library_end();
}

/*
 * Truncate database table
 */
int db_setup() {
	
	int ret;
        char stmt_create[] = "CREATE TABLE IF NOT EXISTS `%s` ("
                        "`Path` varchar(512) default NULL, "
                        "`Base` varchar(512) default NULL, "
                        "`Type` tinyint(1) default NULL, "
                        "`Status` tinyint(1) default NULL, "
                        "`Date` datetime default NULL) "
                        "ENGINE=MyISAM DEFAULT CHARSET=utf8";
        char stmt_trunc[] = "TRUNCATE TABLE `%s`";
	
	/* Allocate memory big enough for querys */
	char *stmt = (char *) malloc((sizeof(char) * strlen(dbtable)) + strlen(stmt_create) - 1);

        /* Create mysql query */
	if(sprintf(stmt, stmt_create, dbtable) < 0) {
		fprintf(stderr, "Error, create database sql\n");
	}

	/* Run mysql query */
	ret = mysql_query(dbconn, stmt);
        
	/* Make sure query was successfull */
	if(ret != 0) {
		fprintf(stderr, "%s\n", mysql_error(dbconn));
		return -1;
        }

	/* Create mysql query */
	if(sprintf(stmt, stmt_trunc, dbtable) < 0) {
		fprintf(stderr, "Error, trunc sql\n");
	}
	
	/* Run mysql query */
	ret = mysql_query(dbconn, stmt);
	
	/* Clean up */
	free(stmt);
	
	/* Make sure query was successfull */
	if(ret != 0) {
		fprintf(stderr, "%s\n", mysql_error(dbconn));
		return -1;
	}
	
	return 0;
}

/*
 * Insert into database
 */
int arch_db_insert(const char *path, const char *filename, const int isdir) {
	
	int ret;
	char *esc_path;
	char *esc_filename;
	long stmt_size;
	long esc_path_size;
	long esc_filename_size;
	
	char date[25];
	time_t curtime;
	struct tm *loctime;
	
	/* 
	 * Allocate memory big enough for query 
	 * dbtable + path + filename + date + 100 (for base query)
	 */
	stmt_size = sizeof(char) * (strlen(dbtable)+
			strlen(path)+strlen(filename)+sizeof(date)+100);
	
	char *stmt = malloc(stmt_size);
	
	/* Allocate memory for escaped path / filename */
	esc_path_size = (sizeof(char) * strlen(path) * 2) + 1;
	esc_path = malloc(esc_path_size);
	
	esc_filename_size = (sizeof(char) * strlen(filename) * 2) + 1;
	esc_filename = malloc(esc_filename_size);
	
	/* For debuging, prints allocated size for malloc's */
#ifdef DB_DEBUG
	fprintf(stderr, "==============================================\n");
	fprintf(stderr, "Dynamic allocated memory in notify_db_insert()\n");
	fprintf(stderr, "Stmt: %li\n", stmt_size);
	fprintf(stderr, "Path: %li\n", esc_path_size);
	fprintf(stderr, "Filename: %li\n", esc_filename_size);
	fprintf(stderr, "==============================================\n");
#endif
	
	
	/* Get date */
	curtime = time(NULL);
	loctime = localtime(&curtime);
	strftime(date, 25, "%Y-%m-%d %H:%M:%S", loctime);
	
	/* Make sure we got a connection
	 * This is prob. not needed. 
	 */
	if(mysql_ping(dbconn) != 0) {
		fprintf(stderr, "Lost connection to database. Could not reconnect.\n");
		return -1;
	}
	
	mysql_real_escape_string(dbconn, esc_path, path, strlen(path));
	mysql_real_escape_string(dbconn, esc_filename, filename, strlen(filename));
	
	/* Create mysql query */
	sprintf(stmt, "INSERT INTO `%s` (`Path`, `Base`, `Type`, `Status`, `Date`) VALUES('%s','%s','%i','0', '%s')", dbtable, esc_path, esc_filename, isdir, date);
	
	/* Run mysql query */
	ret = mysql_query(dbconn, stmt);
	
	/* Clean up */
	free(stmt);
	free(esc_path);
	free(esc_filename);
	
	/* Make sure query was successfull */
	if(ret != 0) {
		fprintf(stderr, "%s\n", mysql_error(dbconn));
		return -1;
	}

#ifdef DB_DEBUG
	
	if(dbthread_id != mysql_thread_id(dbconn)) {
		fprintf(stderr, "Connection was lost. Reconnected. Old_Thread: %li, New_Thread: %li\n", dbthread_id, mysql_thread_id(dbconn));
	}
	
#endif
	
	return 0;
}

/*
 * Delete row in database
 */
int arch_db_delete(const char *path, const char *filename) {
	
	int ret;
	char *esc_path;
	char *esc_filename;
	size_t stmt_size;
	size_t esc_path_size;
	size_t esc_filename_size;
	
	/* 
	 * Allocate memory big enough for query 
	 * dbtable + path + filename + date + 100 (for base query) + \0
	 */
	stmt_size = STRSZ(dbtable) + 
			STRSZ(path) + STRSZ(filename) + 101;
	
	char *stmt = malloc(stmt_size);
	
	/* Allocate memory for escaped path / filename */
	esc_path_size = (sizeof(char) * strlen(path) * 2) + 1;
	esc_path = malloc(esc_path_size);
	
	esc_filename_size = (sizeof(char) * strlen(filename) * 2) + 1;
	esc_filename = malloc(esc_filename_size);
	
	/* For debuging, prints allocated size for malloc's */
#ifdef DB_DEBUG
	fprintf(stderr, "==============================================\n");
	fprintf(stderr, "Dynamic allocated memory in notify_db_delete()\n");
	fprintf(stderr, "Stmt: %i\n", (int)stmt_size);
	fprintf(stderr, "Path: %i\n", (int)esc_path_size);
	fprintf(stderr, "Filename: %i\n", (int)esc_filename_size);
	fprintf(stderr, "==============================================\n");
#endif
	
	/* Make sure we got a connection
	 * This is prob. not needed. 
	 */
	if(mysql_ping(dbconn) != 0) {
		fprintf(stderr, "Lost connection to database. Did reconnection work?\n");
		return -1;
	}
	
	mysql_real_escape_string(dbconn, esc_path, path, strlen(path));
	mysql_real_escape_string(dbconn, esc_filename, filename, strlen(filename));
	
	/* Create mysql query */
	sprintf(stmt, "DELETE FROM `%s` WHERE `Path` LIKE '%s%s%%' OR (`Path` = '%s' AND `Base` = '%s');", dbtable, esc_path, esc_filename, esc_path, esc_filename);
	
	/* Run mysql query */
	ret = mysql_query(dbconn, stmt);
	
	/* Clean up */
	free(stmt);
	free(esc_path);
	free(esc_filename);
	
	/* Make sure query was successfull */
	if(ret != 0) {
		fprintf(stderr, "%s\n", mysql_error(dbconn));
		return -1;
	}
	
#ifdef DB_DEBUG
	
	if(dbthread_id != mysql_thread_id(dbconn)) {
		fprintf(stderr, "Connection was lost. Reconnected. Old_Thread: %li, New_Thread: %li\n", dbthread_id, mysql_thread_id(dbconn));
	}
	
#endif

	return 0;
}
