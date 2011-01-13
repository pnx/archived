
#ifndef __COMPAT_STRING_H
#define __COMPAT_STRING_H

#include <string.h>

#ifdef NO_MEMRCHR
extern void* memrchr(const void *, int, size_t);
#endif

#endif /* __COMPAT_STRING_H */
