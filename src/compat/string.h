
#ifndef __COMPAT_STRING_H
#define __COMPAT_STRING_H

#include <string.h>

#ifdef NO_MEMRCHR
extern void* memrchr(const void *, int, size_t);
#endif

extern size_t memcpy_sb(void*, size_t, const void*, size_t);

#endif /* __COMPAT_STRING_H */
