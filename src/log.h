/* log.h
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */
#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>
#include <stddef.h>

#define LOG_INFO  (1<<0)
#define LOG_WARN  (1<<1) 
#define LOG_CRIT  (1<<2)
#define LOG_DEBUG (1<<3)
#define LOG_ALL   (LOG_INFO | LOG_WARN | LOG_CRIT | LOG_DEBUG)

void init_log(unsigned level, const char *path);

const char* loglvltostr(unsigned level);

void logmsg(unsigned level, const char *fmt, ...);

void logerrno(unsigned level, const char *prefix, int err);

#endif /* __LOG_H */
