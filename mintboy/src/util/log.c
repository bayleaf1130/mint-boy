#include "util/log.h"

#define USE_PTHREADS

#ifdef USE_PTHREADS
    #include <pthread.h>
    #include <unistd.h>
    #define THREAD_TYPE pthread_t
    #define THREAD_CREATE(id, func, param) pthread_create((id), NULL, (func), (param))
    #define THREAD_EXIT(status) pthread_exit((status))
    #define THREAD_JOIN(id) pthread_join((id), (NULL))
    #define THREAD_SLEEP(time) sleep(time)
    #define MUTEX_TYPE pthread_mutex_t
    #define MUTEX_INIT(mutex) pthread_mutex_init((mutex), NULL)
    #define MUTEX_DESTROY(mutex) pthread_mutex_destroy((mutex))
    #define MUTEX_LOCK(mutex) pthread_mutex_lock((mutex))
    #define MUTEX_UNLOCK(mutex) pthread_mutex_unlock((mutex))
    #define COND_TYPE pthread_cond_t
    #define COND_INIT(condition) pthread_cond_init((condition))
    #define COND_DESTROY(condition) pthread_cond_destroy((condition))
    #define COND_WAIT(condition, mutex) pthread_cond_wait((condition), (mutex))
    #define COND_SIGNAL(condition) pthread_cond_signal((condition))
#else
    #include <threads.h>
    #define THREAD_TYPE thrd_t
    #define THREAD_CREATE(id, func, param) thrd_create((id), (func), (param))
    #define THREAD_EXIT(status) thrd_exit((status))
    #define THREAD_JOIN(id) thrd_join((id), (NULL))
    #define THREAD_SLEEP(duration_timespec, remaining_timespec) thrd_sleep((duration_timespec), (remaining_timespec))
    #define MUTEX_TYPE mtx_t
    #define MUTEX_INIT(mutex) mtx_init((mutex), NULL)
    #define MUTEX_DESTROY(mutex) mtx_destroy((mutex))
    #define MUTEX_LOCK(mutex) mtx_lock((mutex))
    #define MUTEX_UNLOCK(mutex) mtx_unlock((mutex))
    #define COND_TYPE cnd_t
    #define COND_INIT(condition) cnd_init((condition))
    #define COND_DESTROY(condition) cnd_destroy((condition))
    #define COND_WAIT(condition, mutex) cnd_wait((condition), (mutex))
    #define COND_SIGNAL(condition) cnd_signal((condition))
#endif

#include <stdbool.h>

typedef struct {
    LogLevel level;
    const char* file;
    int lineno;
    const char* msg;
} LogMessage;

// Circular dynamic array implementation
typedef struct {
    uint32_t size;
    uint32_t capacity;
    uint32_t factor;
    uint32_t max_size;
    LogMessage** messages;
} LogQueue;

typedef struct {
    LogLevel level;
    const char* filename;
    const char* filepath;
    const char* intro;
    size_t maxsize;
    bool colors;
    uint32_t max_queue_size;
    bool stdout;
} LogFormat;

typedef struct {
    LogFormat* format;
    uint32_t id;
    THREAD_TYPE thread_id;
    LogQueue* queue;
} Logger;

// Internal trackers
static Logger** loggers = 0;
static uint32_t loggers_length = 0;

// TODO Implement Log Queue
static LogQueue* CreateQueue(uint32_t initial_size, uint32_t initial_capicity, uint32_t factor, uint32_t max_size) {

}

static void DestroyQueue(LogQueue** queue) {

}

static void Enqueue(LogQueue* queue, LogMessage* message) {

}

static const LogMessage* Dequeue(LogQueue* queue) {
    
}


// TODO Implement Queue Checks with Condition Variables
// TODO Implement RunLogger
static int RunLogger(void* arg) {
    Logger* logger = arg;


}

static uint32_t InitLogger(LogFormat* format) {
    Logger* logger = NULL;

    // Allocate logger memory
    if (!loggers) {
        loggers = malloc(sizeof(Logger*));
        loggers[0] = malloc(sizeof(Logger));
        logger = loggers[loggers_length];
        logger->id = loggers_length;
        loggers_length++;
    } else {

        // Check for free logger space first before reallocing
        bool found_spot = false;
        uint32_t spot = 0;
        for (uint32_t i = 0; i < loggers_length; i++) {
            if (loggers[i] == NULL) {
                found_spot = true;
                spot = i;
                break;
            }
        }

        if (found_spot) {
            loggers[spot] = malloc(sizeof(Logger));
            logger = loggers[spot];
            logger->id = spot;
        } else {
            loggers = realloc(loggers, sizeof(Logger*) * (loggers_length + 1));
            loggers[loggers_length] = NULL;
            loggers[loggers_length] = malloc(sizeof(Logger));
            logger = loggers[loggers_length];
            logger->id = loggers_length;
            loggers_length++;
        }
    }

    // Add format
    logger->format = format;

    // Spin up a thread for the loggers
    THREAD_TYPE thread;
    THREAD_CREATE(&thread, RunLogger, ((void*)logger));

    // Update thread id
    logger->thread_id = thread;

    // Return Id to user
    return logger->id;
}

uint32_t InitFileLogger(LogLevel level, const char* filename, const char* filepath, const char* intro,  size_t maxsize, uint32_t max_queue_size) {
    LogFormat* log_format = &(LogFormat){
            .level = level,
            .filename = filename,
            .filepath = filepath,
            .intro = intro,
            .maxsize = maxsize,
            .colors = false,
            .max_queue_size = max_queue_size,
            .stdout = false
        };

        return InitLogger(log_format);
}

uint32_t InitStdoutLogger(LogLevel level, const char* intro, bool colors, uint32_t max_queue_size) {
    LogFormat* log_format = &(LogFormat){
            .level = level,
            .filename = NULL,
            .filepath = NULL,
            .intro = intro,
            .maxsize = 0,
            .colors = colors,
            .max_queue_size = max_queue_size,
            .stdout = true
        };

        return InitLogger(log_format);
}

uint32_t InitStderrLogger(LogLevel level, const char* intro, bool colors, uint32_t max_queue_size) {
    LogFormat* log_format = &(LogFormat){
            .level = level,
            .filename = NULL,
            .filepath = NULL,
            .intro = intro,
            .maxsize = 0,
            .colors = colors,
            .max_queue_size = max_queue_size,
            .stdout = false
        };
    
    return InitLogger(log_format);
}

uint32_t CloseLogger(uint32_t logger) {
    // Tell the logger its done,
    // Wait until logger thread joins
    // Clean up file handles if any
    for (uint32_t idx = 0; idx < loggers_length; idx++) {
        if (loggers[idx]->id == logger) {
            THREAD_JOIN(loggers[idx]->thread_id);
            free(loggers[idx]);
            loggers[idx] = NULL;
        }
    }
}

uint32_t CloseLoggers() {
    for (uint32_t idx = 0; idx < loggers_length; idx++) {
        if (loggers[idx]) {
            THREAD_JOIN(loggers[idx]->thread_id);
            free(loggers[idx]);
        }
    }

    // Deallocate memory
    free(loggers);
}

// TODO Implement Log
void Log(uint32_t logger, LogLevel level, const char* file, int line, const char* msg, ...) {

}
