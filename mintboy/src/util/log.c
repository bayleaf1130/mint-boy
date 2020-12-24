#include "util/log.h"
#include "util/common.h"

#if defined(__unix__) || defined(linux) || defined(__APPLE__)
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
#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) 
    // TODO Add windows wrapper macros for threading
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

typedef struct {
    LogLevel level;
    char* msg;
    bool done;
} LogMessage;

// Circular dynamic array implementation
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t capacity;
    uint32_t actual_capacity;
    uint32_t size;
    LogMessage** messages;
} LogQueue;

typedef struct {
    LogLevel level;
    uint32_t max_queue_size;
    bool colors;
    FILE* handle;
    bool flush;
} LogFormat;

typedef struct {
    LogFormat format;
    uint32_t id;
    THREAD_TYPE thread_id;
    LogQueue* queue;
} Logger;

#define LOG_POLL_TIME 1U

// Internal trackers
static Logger** loggers = 0;
static uint32_t loggers_length = 0;
static LogMessage TERMINATE = {.done = true};

// TODO Implement Log Queue
static LogQueue* CreateQueue(uint32_t capacity) {
    #if DEBUG
        assert(capacity > 0U);
    #endif
    LogQueue* queue = error_checked_malloc(sizeof(LogQueue));
    memset(queue, 0U, sizeof(*queue));
    queue->capacity = capacity;
    queue->actual_capacity = capacity + 1;
    queue->head = 0U;
    queue->tail = 0U;
    queue->size = 0U;
    queue->messages = error_checked_malloc(sizeof(LogMessage*) * queue->actual_capacity);
    memset(queue->messages, 0U, sizeof(LogMessage*) * queue->actual_capacity);
    return queue;
}

static void DestroyQueue(LogQueue** queue) {
    #if DEBUG
        assert(queue);
        assert(*queue);
    #endif

    // Safer to go over all of them just in case and free shit,
    // The terminate in the thread loop should do this
    for(uint32_t i = 0; i < (*queue)->capacity; i++) {
        if((*queue)->messages[i]) {
            if ((*queue)->messages[i]->msg) {
                free((*queue)->messages[i]->msg);
                (*queue)->messages[i]->msg = NULL;
            }
            free((*queue)->messages[i]);
            (*queue)->messages[i] = NULL;
        }
    }   


    // Free messages then queue itself
    if ((*queue)->messages) {
        free((*queue)->messages);
        (*queue)->messages = NULL;
    }

    // Reset Values
    (*queue)->capacity = (*queue)->actual_capacity = (*queue)->head = (*queue)->tail = 0U;

    if (*queue) {
        free(*queue);
        *queue = NULL;
    }
}

static bool IsQueueFull(LogQueue* queue) {
    return queue->head + 1 == queue->tail;
}

static bool IsQueueEmpty(LogQueue* queue) {
    return queue->head == queue->tail;
}

static bool Enqueue(LogQueue* queue, LogMessage* message) {
    #if DEBUG
        assert(queue);
        assert(message);
    #endif
    
    // Just dont queue if full
    if (IsQueueFull(queue)) {

        // Last message sentinel
        if (message->done) {
            // Add message and advance queue
            queue->messages[queue->head] = message;
            queue->head = (queue->head + 1) % queue->actual_capacity;
            queue->size++;
            return true;
        } 

        // Free up message
        if (message->msg) {
            free(message->msg);
            message->msg = NULL;
        }
        free(message);
        return false;
      
    }

    // Add message and advance queue
    queue->messages[queue->head] = message;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size++;
    return true;
}

static LogMessage* Dequeue(LogQueue* queue) {
    #if DEBUG
        assert(queue);
    #endif

    // Do not block
    if(IsQueueEmpty(queue)) return NULL;
    LogMessage* message = queue->messages[queue->tail];
    queue->messages[queue->tail] = NULL;

    // Last message sentinel
    if (message->done) {
        queue->tail = (queue->tail + 1) % queue->actual_capacity;
    } else {
        queue->tail = (queue->tail + 1) % queue->capacity;
    }

    queue->size--;
    return message;
    
}


static void LogIt(Logger* logger, LogMessage* message) {
    #if DEBUG
        assert(logger);
        assert(message);
        assert(message->msg);
    #endif

    // If level is okay
    if (message->level >= (logger->format).level) {
        fputs(message->msg, (logger->format).handle);
        fputs("\n", (logger->format).handle);

        if ((logger->format).flush) {
            fflush((logger->format).handle);
        }
    }

}


static void* RunLogger(void* arg) {
    #if DEBUG
        assert(arg);
    #endif
    Logger* logger = arg;
    #if DEBUG
        assert(logger->queue);
    #endif

    while(true) {
        LogMessage* message = Dequeue(logger->queue);

        // Wait a bit
        if (!message) {
            THREAD_SLEEP(LOG_POLL_TIME);
            continue;
        }

        // Done at this point
        if (message->done) {
            break;
        }

        // Do something with message
        LogIt(logger, message);

          // Clean up message
        if (message->msg) {
            free(message->msg);
            message->msg = NULL;
        }

        // Clean up message
        free(message);
    }

    DestroyQueue(&logger->queue);
    return EXIT_SUCCESS;
}

static const char* StringFromLevel(LogLevel level) {
    const char* msg = NULL;
    switch(level) {
        case LOG_DEBUG:
            msg = "DEBUG";
            break;
        case LOG_INFO:
            msg = "INFO";
            break;
        case LOG_WARN:
            msg = "WARN";
            break;
        case LOG_ERROR:
            msg = "ERROR";
            break;
        case LOG_FATAL:
            msg = "FATAL";
            break;
        default:
            msg = "UNKNOWN";
            break;
    }

    return msg;
}


uint32_t InitLogger(FILE* handle, LogLevel level, uint32_t max_queue_size, bool flush) {
    #if DEBUG
        assert(handle);
    #endif
    Logger* logger = NULL;

    LogFormat log_format = (LogFormat){
        .level = level,
        .max_queue_size = max_queue_size,
        .handle = handle,
        .colors = false,
        .flush = flush
    };

    // Allocate logger memory
    if (!loggers) {
        loggers = error_checked_malloc(sizeof(Logger*));
        loggers[0] = error_checked_malloc(sizeof(Logger));
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
            loggers[spot] = error_checked_malloc(sizeof(Logger));
            logger = loggers[spot];
            logger->id = spot;
        } else {
            loggers = error_checked_realloc(loggers, sizeof(Logger*) * (loggers_length + 1));
            loggers[loggers_length] = NULL;
            loggers[loggers_length] = error_checked_malloc(sizeof(Logger));
            logger = loggers[loggers_length];
            logger->id = loggers_length;
            loggers_length++;
        }
    }

    // Add format
    logger->format = log_format;
    LogQueue* queue = CreateQueue(logger->format.max_queue_size);
    logger->queue = queue;

    // Spin up a thread for the loggers
    THREAD_TYPE thread;
    THREAD_CREATE(&thread, RunLogger, ((void*)logger));

    // Update thread id
    logger->thread_id = thread;

    // Return Id to user
    return logger->id;
}


void CloseLogger(uint32_t logger) {

    for (uint32_t idx = 0; idx < loggers_length; idx++) {
        if (loggers[idx]->id == logger) {
            // Wait for messages to end
            Enqueue(loggers[idx]->queue, &TERMINATE);
            THREAD_JOIN(loggers[idx]->thread_id);
            free(loggers[idx]);
            loggers[idx] = NULL;
        }
    }
}

void CloseLoggers() {
    for (uint32_t idx = 0; idx < loggers_length; idx++) {
        if (loggers[idx]) {
            // Wait for messages to end
            Enqueue(loggers[idx]->queue, &TERMINATE);
            THREAD_JOIN(loggers[idx]->thread_id);
            free(loggers[idx]);
            loggers[idx] = NULL;
        }
    }

    // Deallocate memory
    free(loggers);
}

void Log(uint32_t logger_id, LogLevel level, const char* msg) {
    #ifdef DEBUG
        assert(logger_id < loggers_length);
        assert(msg);
    #endif

    // Get associated logger from table
    Logger* logger = loggers[logger_id];
    #ifdef DEBUG
        assert(logger);
    #endif

    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;
    const char* level_string = StringFromLevel(level);
    uint32_t level_size = strlen(level_string);
    char buf[64];
    time_t current_time = time(NULL);
    struct tm* ct = localtime(&current_time);
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ct)] = '\0';
    LogMessage* message = error_checked_malloc(sizeof(LogMessage));
    
    // Create message here
    formatted_msg = error_checked_malloc(strlen(msg) + strlen(buf) + level_size + padding + 1);
    sprintf(formatted_msg, "[%s] %s %s", buf, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    Enqueue(logger->queue, message);
}

void Log2(uint32_t logger_id, LogLevel level, char* msg, bool freeit) {
    #ifdef DEBUG
        assert(logger_id < loggers_length);
        assert(msg);
    #endif

    // Get associated logger from table
    Logger* logger = loggers[logger_id];
    #ifdef DEBUG
        assert(logger);
    #endif

    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;
    const char* level_string = StringFromLevel(level);
    uint32_t level_size = strlen(level_string);
    char buf[64];
    time_t current_time = time(NULL);
    struct tm* ct = localtime(&current_time);
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ct)] = '\0';
    LogMessage* message = error_checked_malloc(sizeof(LogMessage));
    
    // Create message here
    formatted_msg = error_checked_malloc(strlen(msg) + strlen(buf) + level_size + padding + 1);
    sprintf(formatted_msg, "[%s] %s %s", buf, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    // Free original message
    if (freeit) {
        free(msg);
    }

    Enqueue(logger->queue, message);
}
