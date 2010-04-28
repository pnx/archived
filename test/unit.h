
#ifndef _UNIT_H

#define _UNIT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define assert_string(a, b) \
	assert(strcmp((char*)a, (char*)b) == 0)

void utest_init_RNG();

char* utest_ran_string(size_t size);

#endif /* _UNIT_H */
