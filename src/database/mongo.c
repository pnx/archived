/* database/mongo.c - mongodb driver
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *   This driver uses the official mongodb C driver API.
 *   however error handling is somewhat weak, looks like this is a design
 *   choice of mongodb and not much can be done about that.
 */

#include <string.h>
#include <time.h>
#include <mongo/mongo.h>
#include <ini/iniparser.h>
#include "../path.h"
#include "../database.h"

static struct {
    mongo_connection conn;
    mongo_connection_options opts;
    char ns[255];
} db;

static void coll_create_index() {

    bson b;
    bson_buffer buf;

    bson_buffer_init(&buf);
    bson_append_int(&buf, "Path", 1);
    bson_append_int(&buf, "File", 1);
    bson_from_buffer(&b, &buf);

    mongo_create_index(&db.conn, db.ns, &b, 0, NULL);

    bson_destroy(&b);
}

static void coll_clear() {

    bson cond;
    bson_empty(&cond);
    mongo_remove(&db.conn, db.ns, &cond);
    bson_destroy(&cond);
}

int database_init(dictionary *conf) {

    char *confdb = iniparser_getstring(conf, "mongo:database", NULL);
    char *confcoll = iniparser_getstring(conf, "mongo:collection", NULL);

    if (!confcoll) {
        fprintf(stderr, "mongo: missing 'collection' in configuration\n");
        return -1;
    }
    if (!confcoll) {
        fprintf(stderr, "mongo: missing 'database' in configuration\n");
        return -1;
    }

    strncpy(db.opts.host, iniparser_getstring(conf, "mongo:host", "127.0.0.1"), sizeof(db.opts.host));
    db.opts.port = iniparser_getint(conf, "mongo:port", 27017);

    mongo_conn_return status = mongo_connect(&db.conn, &db.opts);

    if (status != mongo_conn_success) {

        char *err;

        switch (status) {
        case mongo_conn_bad_arg :
            err = "Bad arguments";
            break;
        case mongo_conn_no_socket :
            err = "No socket";
            break;
        case mongo_conn_fail:
            err = "Connection failed";
            break;
        case mongo_conn_not_master:
            err = "Not master\n";
            break;
        default:
            err = "Unkown error";
        }

        fprintf(stderr, "mongo: %s (%s:%i)\n", err, db.opts.host, db.opts.port);
        return -1;
    }

    /* do auth if these values are precent */
    char *user = iniparser_getstring(conf, "mongo:username", NULL);
    char *pass = iniparser_getstring(conf, "mongo:password", NULL);

    if (user && pass) {

        bson_bool_t rc = mongo_cmd_authenticate(&db.conn, confdb, user, pass);

        if (!rc) {
            fprintf(stderr, "mongo: can't authenticate\n");
            return -1;
        }
    }

    /* create the namespace string */
    snprintf(db.ns, sizeof(db.ns), "%s.%s", confdb, confcoll);

    /* prepare collection */
    coll_clear();
    coll_create_index();

    return 0;
}

int database_insert(const char *path, const char *filename, const int isdir) {

    bson b;
    bson_buffer buf;

    bson_buffer_init(&buf);
    bson_append_new_oid(&buf, "_id");
    bson_append_string(&buf, "Path", path);
    bson_append_string(&buf, "File", filename);
    bson_append_int(&buf, "Type", isdir != 0);
    bson_append_time_t(&buf, "Date", time(NULL));
    bson_from_buffer(&b, &buf);

    mongo_insert(&db.conn, db.ns, &b);

    bson_destroy(&b);

    return 0;
}

int database_delete(const char *path, const char *filename) {

    bson cond;
    bson_buffer buf;
    char *fpath = path_normalize(path, filename, 1);

    if (!fpath)
        return -1;

    bson_buffer_init(&buf);
    bson_append_string(&buf, "Path", path);
    bson_append_string(&buf, "File", filename);
    bson_from_buffer(&cond, &buf);

    mongo_remove(&db.conn, db.ns, &cond);
    bson_destroy(&cond);

    bson_buffer_init(&buf);
    bson_append_regex(&buf, "Path", fpath, "");
    bson_from_buffer(&cond, &buf);

    mongo_remove(&db.conn, db.ns, &cond);

    bson_destroy(&cond);
    free(fpath);

    return 0;
}

int database_close() {

    mongo_destroy(&db.conn);

    return 0;
}
