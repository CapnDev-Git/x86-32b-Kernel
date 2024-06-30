#ifndef STDLIB_H_
#define STDLIB_H_

#include <stddef.h>

void *malloc(size_t size);

void *realloc(void *ptr, size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

static inline int abs(int j) { return j > 0 ? j : -j; }

#endif /* !STDLIB_H_ */
