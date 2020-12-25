#ifndef MINTBOY_ARGUMENTS_H
#define MINTBOY_ARGUMENTS_H

#include <stdbool.h>

typedef struct {
    bool debug;
    const char* rom_name;
} Arguments;

Arguments ParseArgs(int argc, char** argv);

#endif
