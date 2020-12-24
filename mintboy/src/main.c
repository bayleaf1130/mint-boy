#include "cpu/cpu.h"
#include "util/log.h"
#include "util/arguments.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: ./mintboy [--debug] <rom>");
        return EXIT_FAILURE;
    }

    uint32_t logger_id = 0;
    logger_id = InitLogger(stdout, LOG_DEBUG, 6U, true);

    // Get rid of compiler warning
    fputs(argv[0], stdout);
    fputs("\n", stdout);

    // Parse Args
    // Arguments args = ParseArgs(argc, argv);
    Log(logger_id, LOG_DEBUG, "Starting MintBoy....");
    

    // Clean up resources before exiting the program
    CloseLoggers();

    return EXIT_SUCCESS;
}