#ifndef LOGGO_LOG_H
#define LOGGO_LOG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Log Levels
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

typedef int (*CloseStreamHandler)(FILE*);
typedef int (*CloseDescriptorHandler)(int);

// The user controls the format
typedef struct {
    LogLevel level;
    uint32_t max_queue_size;
    bool colors;
    bool flush;
    const char* time_format;
    const char* linesep;
    const char* linebeg;
} UserLogFormat;

/* 
 * Init Logger in its own thread.
 * Create the logger or the queue.
 * A handle is returned on success or -1 on failure.
 * A format is required to have a handle to output logs too at the very minimum
 */
int32_t InitStreamLogger(UserLogFormat* user_format, FILE* handle, CloseStreamHandler close_handler);


/* 
 * Init Logger in its own thread.
 * Create the logger or the queue.
 * A handle is returned on success or -1 on failure.
 * A format is required to have a fd to output logs too at the very minimum
 */
int32_t InitDescriptorLogger(UserLogFormat* user_format, int fd, CloseDescriptorHandler close_handler);


/* 
 * Stop Logger threads and clean up handles
 */
void ShutdownLoggers();

/* 
 * Pass messages to the log queue, 
 * the thread will then read and write them to the specified output 
 */
void Log(int32_t logger_id, LogLevel level, const char* msg);
void Log2(int32_t logger_id, LogLevel level, char* msg, bool free_string);


#endif
