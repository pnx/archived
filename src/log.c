/* log.c
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "strbuf.h"
#include "log.h"

static FILE *logfd = NULL;
static char *file = NULL;
static char *logname = NULL;

static struct {
    const char *name;
    unsigned mask;
} levels[] = {
    { "INFO",     LOG_INFO  },
    { "WARNING",  LOG_WARN  },
    { "CRITICAL", LOG_CRIT  },
    { "DEBUG",    LOG_DEBUG },
    { NULL,       0         }
};

static int mask = 0;

/* check (and change) if we need to roll logging file */
static void checklog() {

    if (file) {
        time_t t;
        char name[15];

        time(&t);
        strftime(name, sizeof(name), "%Y-%m-%d.log", localtime(&t));

        /* update the name if we have changed date */
        if (strncmp(logname, name, sizeof(name))) {
            memcpy(logname, name, sizeof(name));

            if (logfd && logfd != stderr)
                fclose(logfd);
            logfd = fopen(file, "a");
        }
    }

    if (!logfd)
        logfd = stderr;
}

/* writes the information about a logg message (timestamp and level) */
static void writeinfo(unsigned level) {

    time_t tnow;
    char buf[512], *ptr;

    time(&tnow);
    ptr = buf + strftime(buf, sizeof(buf), "[%H:%M:%S] ", localtime(&tnow));
    ptr += sprintf(ptr, "%-10s ", loglvltostr(level));

    fputs(buf, logfd);
}

static int validmask(unsigned x, unsigned unique) {

    /* check if only one bit is set */
    if (unique && (x & (x - 1)) != 0)
        return 0;

    /* make sure we don't have any unused bits */
    return (x & ~LOG_ALL) == 0;
}

void init_log(unsigned level, const char *path) {

    if (!validmask(level, 0))
        die("init_log: unknown level: %x\n", level);

    mask = level;

    if (path) {
        strbuf_t sb = STRBUF_INIT;
        size_t nameoffset;

        strbuf_append_str(&sb, path);
        strbuf_term(&sb, '/');
        nameoffset = sb.len;

        strbuf_append_repeat(&sb, 0, 14);

        if (file)
            xfree(file);

        file = strbuf_release(&sb);
        logname = file + nameoffset;
    } else {
        logname = file = NULL;
    }
}

const char* loglvltostr(unsigned level) {

    int i;

    for(i=0; levels[i].name; i++) {

        if (level & levels[i].mask)
            return levels[i].name;
    }
    return "UNKNOWN";
}

unsigned logstrtolvl(const char *str) {

    int i;

    if (strcmp(str, "ALL") == 0)
        return LOG_ALL;

    for(i=0; levels[i].name; i++) {

        if (strcmp(str, levels[i].name) == 0)
            return levels[i].mask;
    }
    return 0;
}

void logmsg(unsigned level, const char *fmt, ...) {

	va_list vl;

    if (!validmask(level, 1))
        die("log: invalid level: %x\n", level);

    if (level & ~mask)
        return;

    checklog();
    writeinfo(level);
    va_start(vl, fmt);
    vfprintf(logfd, fmt, vl);
    va_end(vl);
    fputc('\n', logfd);
    fflush(logfd);
}

void logerrno(unsigned level, const char *prefix, int err) {

	char *str = strerror(err);
    
	if (str && level & mask) {

        if (!validmask(level, 1))
            die("logerrno: invalid level: %x\n", level);

        checklog();
        writeinfo(level);
        if (prefix && *prefix)
            fprintf(logfd, "%s: ", prefix);
        fputs(str, logfd);
        fputc('\n', logfd);
        fflush(logfd);
	}
}
