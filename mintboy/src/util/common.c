#include "util/common.h"
#include <stdlib.h>
#include <stdio.h>

void* error_checked_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because malloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* error_checked_realloc(void* ptr, size_t size) {
    void* result = realloc(ptr, size);
    if (!result) {
        fprintf(stderr, "[ERROR] Exiting because realloc failed...");
        exit(EXIT_FAILURE);
    }
    return result;
}
