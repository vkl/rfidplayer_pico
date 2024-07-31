#ifndef MCHECK_H
#define MCHECK_H

#include <stdlib.h>
#include <stddef.h>

#define MMALLOC measureMalloc
#define MFREE measureFree

extern size_t measureMallocCount;
extern size_t measureFreeCount;
extern size_t measureMallocBytes;

void *measureMalloc(size_t bytes);
void measureFree(void * ptr);

#endif // MCHECK_H