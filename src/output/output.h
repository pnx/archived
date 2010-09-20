/* arch/output.h - output API
 *
 *   Copyright (C) 2010  Fredric Nilsson <fredric@fiktivkod.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 */

#ifndef __OUTPUT_H

#define __OUTPUT_H

#include "../ini/dictionary.h"
#include "../notify/event.h"

int output_init(dictionary *config);

int output_process(notify_event *event);

char *output_error(int error);

int output_exit(void);


#endif
