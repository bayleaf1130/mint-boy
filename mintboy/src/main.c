#include "cpu/cpu.h"
#include "util/log.h"
#include "util/arguments.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: ./mintboy [--debug] <rom>");
        return EXIT_FAILURE;
    }

    uint32_t logger = 0;

    #ifdef DEBUG
        logger = InitStderrLogger(DEBUG, "(level) (msg)", true, 1024U);
    #else
        logger = InitFileLogger(
            WARN, 
            "mintboy-debug.log", 
            ".", 
            "(time) (level) (file) (line) (msg)",
            0xFFFFU,
            1024U
        );
    #endif

    // Parse Args
    Arguments args = ParseArgs(argc, argv);

    // Clean up resources before exiting the program
    CloseLoggers();

    return EXIT_SUCCESS;
}