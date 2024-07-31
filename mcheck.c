#include "mcheck.h"

// Path: mcheck.c
void *measureMalloc(size_t bytes){
	void * ptr = malloc(bytes);
	size_t *sizePtr=((size_t *) ptr)-1;
	measureMallocBytes += *sizePtr;
	measureMallocCount++;
	return ptr;
}

void measureFree(void * ptr){
	size_t *sizePtr=((size_t *) ptr)-1;
	measureFreeCount++;
	measureMallocBytes -= *sizePtr;
	free(ptr);
}
