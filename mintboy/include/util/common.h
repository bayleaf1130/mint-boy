#ifndef MINTBOY_COMMON_H
#define MINTBOY_COMMON_H

#include <stdlib.h>
#include <assert.h>

void* error_checked_malloc(size_t size);
void* error_checked_realloc(void* ptr, size_t size);

#endif