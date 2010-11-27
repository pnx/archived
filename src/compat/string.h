
#include <string.h>

#ifdef NO_MEMRCHR
extern void* memrchr(const void *, int, size_t);
#endif
