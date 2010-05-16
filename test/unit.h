
#ifndef _UNIT_H

#define _UNIT_H

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
inline void __assert_str(char *file, int line, char *func, char *a, char *b) {

    if (a == NULL || b == NULL)
        __uexit(file, line, func, "a or b is null\n", NULL);

    if (strcmp(a, b) != 0)
        __uexit(file, line, func, "\"%s\" != \"%s\"\n", a, b);
    
}

#define assert_string(a, b) __assert_str(__FILE__, __LINE__, __FUNCTION__, a, b)

void utest_init_RNG();

char* utest_ran_string(size_t size);

#endif /* _UNIT_H */
