
#ifndef _UNIT_H

#define _UNIT_H 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define __uexit(file, line, func, fmt, ...) \
    do { \
        fprintf(stderr, "ASSERT %s in %s(%i): " fmt, func, file, line, __VA_ARGS__); \
        exit(1); \
    } while(0)

/* internal function. assert_* macros below expands to this */
void __assert_str(const char *file, int line, const char *func, const char *a, const char *b);

#define assert_string(a, b) __assert_str(__FILE__, __LINE__, __FUNCTION__, a, b)

void utest_init_RNG();

char* utest_ran_string(size_t size);

#endif /* _UNIT_H */
