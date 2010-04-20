
#ifndef _INDEXER_H

#define _INDEXER_H

void indexer_register(const char *base, const char *name);

void indexer_run();

inline int indexer_pending();

#endif /* _INDEXER_H */
