
#ifndef _UNIT_H

#define _UNIT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define utest_exit(fmt, ...) \
    fprintf(stderr, fmt, __VA_ARGS__); \
    exit(1)

#define utest_string(a, b) \
	(strcmp(a, b) == 0 ? \
		(void) 0 : \
		utest_exit("ASSERT: \"" #expr "\" at %s:%i\n", __FILE__, __LINE__))
        
        
#endif /* _UNIT_H */
