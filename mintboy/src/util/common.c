#define MINT_LOGGO_IMPLEMENTATION
#include "mint_loggo.h"
#include "common.h"

#include <stdint.h>
#include <stdio.h>

void logger_init() {
    G_LOGGER_NAME = "mintboy_logger";
    int32_t logger = Mint_Loggo_CreateLogger(G_LOGGER_NAME, 
                        &(Mint_Loggo_LogFormat){.colors=true, .level=MINT_LOGGO_LEVEL_DEBUG, .flush=true, .linebeg="[LOG STDOUT]", .linesep="\n"},
                        NULL);


    if (logger == -1) {
        Mint_Loggo_DeleteLoggers();
        fprintf(stderr, "Could not init logger..... Exiting");
        exit(EXIT_FAILURE);
    }
}

void logger_cleanup() {
    Mint_Loggo_DeleteLoggers();
}

