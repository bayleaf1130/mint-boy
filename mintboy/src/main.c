#include "cpu/cpu.h"

#define LOGGO_IMPLEMENTATION
#define LOGGO_SHORT_NAMES
#define LOGGO_USE_HELPERS
#include "util/log.h"
#include "util/arguments.h"

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

    int32_t id = CreateLogger("stdout", 
                            &(LoggoLogFormat){.colors=true, .level=LOGGO_LEVEL_DEBUG, .flush=true, .time_format="%Y-%M-%D", .linebeg="[LOG STDOUT]", .linesep="\n"},
                            NULL);

    int32_t id2 = CreateLogger("stderr", 
                            &(LoggoLogFormat){.colors=true, .level=LOGGO_LEVEL_DEBUG, .flush=true, .time_format="%Y-%M-%D", .linebeg="[LOG STDERR]", .linesep="\n"},
                            &(LoggoLogHandler){.handle=stderr, .write_handler=StreamWrite});

    if (id == -1 || id2 == -1) {
        DeleteLoggers();
        fprintf(stderr, "Could not init logger..... Exiting");
        exit(EXIT_FAILURE);
    }

    LOG_DEBUG("stdout", "Hello Debug");
    LOG_INFO("stdout", "Hello Info");
    LOG_WARN("stdout", "Hello Warn");
    LOG_ERROR("stdout", "Hello Error");
    LOG_FATAL("stdout", "Hello Fatal");

    LOG_DEBUG("stderr", "Hello D");
    LOG_INFO("stderr", "Hello I");
    LOG_WARN("stderr", "Hello W");
    LOG_ERROR("stderr", "Hello E");
    LOG_FATAL("stderr", "Hello F");


    // Parse Args
    Arguments args = ParseArgs(argc, argv);
    fprintf(stdout, "[Args Parsed]\n\tDebug: %d\n\tRom: %s\n\t", args.debug, args.rom_name);

    LOG_DEBUG("stdout", "DONE");
    DeleteLoggers();

    return EXIT_SUCCESS;
}
