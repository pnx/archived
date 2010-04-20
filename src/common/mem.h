
#ifndef _COMMON_MEM_H

#define _COMMON_MEM_H

#include <malloc.h>

#define sfree(x) \
	if (x != NULL) \
		free(x)

#endif /* _COMMON_MEM_H */
