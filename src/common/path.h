
#ifndef __COMMON_PATH_H

#define __COMMON_PATH_H

#include <stddef.h>

int is_abspath(const char *path);

size_t pathlen(const char *path);

char* fmt_path(const char *base, const char *name, unsigned char dir);

char* basename(char *path);

char* dirname(char *path);

#endif /* __COMMON_PATH_H */
