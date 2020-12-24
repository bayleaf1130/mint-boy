#ifndef MINTBOY_LOG_H
#define MINTBOY_LOG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

/* 
Init Logger in its own thread, return a handle for the logger
*/
uint32_t InitLogger(FILE *handle, LogLevel level, uint32_t max_queue_size, bool flush);

/* 
Stop Logger thread and clean up handles
This just releases a handle. 
CloseLoggers still needs to be called.
*/
void CloseLogger(uint32_t logger);

/* 
Stop Logger threads and clean up handles
*/
void CloseLoggers();

/* 
Pass messages to the log queue, 
the thread will then read and write them to the specified output 
*/
void Log(uint32_t logger_id, LogLevel level, const char* msg);
void Log2(uint32_t logger_id, LogLevel level, char* msg, bool freeit);


#endif