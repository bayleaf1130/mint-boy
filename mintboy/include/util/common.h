#ifndef MINTBOY_COMMON_H
#define MINTBOY_COMMON_H

#define MINT_LOGGO_USE_HELPERS
#include "mint_loggo.h"

#define MINTBOY_UNUSED(arg) (void)arg;

// Macros
#ifdef MINTBOY_DEBUG
    #include <assert.h>
    #define SAFETY_ASSERT(condition) do { assert(condition); } while(0)
#else
    #define SAFETY_ASSERT(condition) do {} while(0)
#endif

const char* G_LOGGER_NAME;
void logger_init();
void logger_cleanup();

#endif // MINTBOY_COMMON_H