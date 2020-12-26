#include "cpu/cpu.h"
#include "util/log.h"
#include "util/arguments.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int dummy_stdout_close(FILE* file) {
    fputs("IM DONE", stdout);
    return *(int*)file;
}

int dummy_close(int fd) {
    // fputs("IM DONE fd", stdout);
    return fd;
}

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: ./mintboy [--debug] <rom>");
        return EXIT_FAILURE;
    }

    FILE* file = fopen("log.txt", "w");

    int32_t logger_id = 0;
    int32_t raw_logger_id = 0;
    logger_id = InitStreamLogger(&(UserLogFormat){
        .max_queue_size=24U,
        .level=LOG_DEBUG,
        .flush=true,
        .time_format="%Y-%m-%d",
        .linesep="\n",
        .linebeg="====> [LOG STREAM] ",
        .colors=false,
    }, file, NULL);

    raw_logger_id = InitStreamLogger(&(UserLogFormat){
        .max_queue_size=24U,
        .level=LOG_DEBUG,
        .flush=true,
        .time_format="%Y-%m-%d",
        .linesep="\n",
        .linebeg="====> [LOG STREAM 2] ",
        .colors=true,
    }, stdout, dummy_stdout_close);

    // Get rid of compiler warning
    fputs("\n", stdout);

    // Parse Args
    // Arguments args = ParseArgs(argc, argv);
    Log(logger_id, LOG_DEBUG, "Starting MintBoy1....");
    Log(raw_logger_id, LOG_INFO, "Starting MintBoy2....");
    Log(logger_id, LOG_WARN, "Starting MintBoy3....");
    Log(raw_logger_id, LOG_ERROR, "Starting MintBoy4....");
    Log(logger_id, LOG_FATAL, "Starting MintBoy5....");

    Log(raw_logger_id, LOG_DEBUG, "Starting MintBoy6....");
    Log(logger_id, LOG_INFO, "Starting MintBoy7....");
    Log(raw_logger_id, LOG_WARN, "Starting MintBoy8....");
    Log(logger_id, LOG_ERROR, "Starting MintBoy9....");
    Log(raw_logger_id, LOG_FATAL, "Starting MintBoy10....");

    ShutdownLoggers();


    FILE* file2 = fopen("log2.txt", "w");
    if(!file2) fputs("uhh..", stdout);

    uint32_t another_id = InitStreamLogger(&(UserLogFormat){
        .max_queue_size=24U,
        .level=LOG_DEBUG,
        .flush=true,
        .time_format="%Y-%m-%d",
        .linesep="\n",
        .linebeg="====> [LOG SDF] ",
        .colors=false,
    }, file2, NULL);

    // Get rid of compiler warning
    fputs("\n", stdout);
    fputs(argv[0], stdout);


    Log(another_id, LOG_DEBUG, "Dum1....");
    Log(another_id, LOG_INFO, "Dum2....");
    Log(another_id, LOG_WARN, "Dum3....");
    Log(another_id, LOG_ERROR, "Dum4....");
    Log(another_id, LOG_FATAL, "Dum5....");


    // Parse Args
    // Arguments args = ParseArgs(argc, argv);
  

    ShutdownLoggers();

    return EXIT_SUCCESS;
}
