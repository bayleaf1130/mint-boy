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

    uint32_t log_handle = 0;
    LogFormat* log_format = NULL;

    #ifdef DEBUG
        log_format = &(LogFormat){
            .level = DEBUG,
            .filename = NULL,
            .intro = "(level) (msg)"
        };
    #else
        log_format = &(LogFormat){
            .level = WARN,
            .filename = "mintboy-debug.log",
            .intro = "(time) (level) (file) (line) (msg)",
            .maxsize = 0xFFFFF // 1 MB
        };
    #endif

    // Init Logging before anythhing else
    log_handle = InitLogger(log_format);

    // Parse Args
    Arguments args = ParseArgs(argc, argv);

    return EXIT_SUCCESS;
}