#ifndef MINTBOY_LOG_H
#define MINTBOY_LOG_H

#include <stdlib.h>
#include <stdint.h>

typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
} LogLevel;

typedef struct {
    LogLevel level;
    const char* filename;
    const char* intro;
    size_t maxsize;
} LogFormat;

/* 
Init Logger in its own thread 
*/
uint32_t InitLogger(LogFormat* format);

/* 
Pass messages to the log queue, 
the thread will then read and write them to the specified output 
*/
void Log(uint32_t log_handle, LogLevel level, const char* file, int line, const char* msg, ...);



#endif