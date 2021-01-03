#include "cpu.h"
#include "arguments.h"
#include "common.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>


int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: ./mintboy [--debug] <rom>");
        return EXIT_FAILURE;
    }

    logger_init();


    LOG_DEBUG(G_LOGGER_NAME, "Hello Debug");
    LOG_INFO(G_LOGGER_NAME, "Hello Info");
    LOG_WARN(G_LOGGER_NAME, "Hello Warn");
    LOG_ERROR(G_LOGGER_NAME, "Hello Error");
    LOG_FATAL(G_LOGGER_NAME, "Hello Fatal");


    // Parse Args
    Arguments args = ParseArgs(argc, argv);
    char info[128U];
    snprintf(info, sizeof(info), "[ARGS] Debug: %s, Rom: %s", args.debug ? "True" : "False", args.rom_name ? args.rom_name : "None");
    LOG_INFO(G_LOGGER_NAME, info);

    // Call this before close
    logger_cleanup();
    return EXIT_SUCCESS;
}
