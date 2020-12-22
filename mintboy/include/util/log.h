#ifndef MINTBOY_LOG_H
#define MINTBOY_LOG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
} LogLevel;

/* 
Init Logger in its own thread, return a handle for the logger
*/
uint32_t InitFileLogger(LogLevel level, const char* filename, const char* filepath, const char* intro,  size_t maxsize, uint32_t max_queue_size);

/* 
Init Logger in its own thread, return a handle for the logger
*/
uint32_t InitStdoutLogger(LogLevel level, const char* intro, bool colors, uint32_t max_queue_size);

/* 
Init Logger in its own thread, return a handle for the logger
*/
uint32_t InitStderrLogger(LogLevel level, const char* intro, bool colors, uint32_t max_queue_size);

/* 
Stop Logger thread and clean up handles
This just releases a handle. 
CloseLoggers still needs to be called.
*/
uint32_t CloseLogger(uint32_t logger);

/* 
Stop Logger threads and clean up handles
*/
uint32_t CloseLoggers();

/* 
Pass messages to the log queue, 
the thread will then read and write them to the specified output 
*/
void Log(uint32_t logger, LogLevel level, const char* file, int line, const char* msg, ...);



#endif