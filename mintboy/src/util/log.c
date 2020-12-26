#include "util/log.h"
#include "util/common.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

// Threading
#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(LOGGO_USE_POSIX)
    #include <pthread.h>
    #include <unistd.h>
    #define THREAD_LOCAL __thread
    #define THREAD_TYPE pthread_t
    #define THREAD_CREATE(id, func, param) pthread_create((id), NULL, (func), (param))
    #define THREAD_JOIN(id) pthread_join((id), (NULL))
    #define MUTEX_TYPE pthread_mutex_t
    #define MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
    #define MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
    #define MUTEX_LOCK(mutex) pthread_mutex_lock(&(mutex))
    #define MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&(mutex))
    #define COND_TYPE pthread_cond_t
    #define COND_INIT(condition) pthread_cond_init(&(condition), NULL)
    #define COND_DESTROY(condition) pthread_cond_destroy(&(condition))
    #define COND_WAIT(condition, mutex) pthread_cond_wait(&(condition), &(mutex))
    #define COND_SIGNAL(condition) pthread_cond_signal(&(condition))
    #define RED      "\x1B[31m"
    #define GREEN    "\x1B[32m"
    #define YELLOW   "\x1B[33m"
    #define BLUE     "\x1B[34m"
    #define MAGENTA  "\x1B[35m"
    #define CYAN     "\x1B[36m"
    #define WHITE    "\x1B[37m"
    #define RESET    "\033[0m"
    #define DEFAULT_FILE_CLOSE_HANDLER fclose
    #define DEFAULT_DESCRIPTOR_CLOSE_HANDLER close
    #define DESCRIPTOR_WRITE(text, fd) write((fd), (text), strlen(text))
    #define STREAM_WRITE(text, handle) fputs((text), (handle))
#elif defined(_WIN32) || defined(LOGGO_USE_WINDOWS)
    #include <io.h>
    #include <Windows.h>
    #define THREAD_LOCAL __thread
    #define THREAD_TYPE LPDWORD
    #define THREAD_CREATE(id, func, param) CreateThread(NULL, 0, func, param, 0, id)
    #define THREAD_JOIN(id) WaitForSingleObject((id), INFINITE)
    #define MUTEX_TYPE LPCRITICAL_SECTION
    #define MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
    #define MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
    #define MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
    #define MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
    #define COND_TYPE PCONDITION_VARIABLE
    #define COND_INIT(condition) InitializeConditionVariable((condition))
    #define COND_DESTROY(condition) DeleteConditionVariable((condition))
    #define COND_WAIT(condition, mutex) SleepConditionVariableCS((condition), (mutex), INFINITE)
    #define COND_SIGNAL(condition) WakeConditionVariable((condition))
    // TODO Fix this
    #define RED      ""
    #define GREEN    ""
    #define YELLOW   ""
    #define BLUE     ""
    #define MAGENTA  ""
    #define CYAN     ""
    #define WHITE    ""
    #define RESET    ""
    #define DEFAULT_FILE_CLOSE_HANDLER fclose
    #define DEFAULT_DESCRIPTOR_CLOSE_HANDLER _close
    #define DESCRIPTOR_WRITE(text, fd) _write((fd), (text), strlen(text))
    #define STREAM_WRITE(text, handle) fputs((text), (handle))
    // TODO Add windows wrapper macros for threading
#endif


////////////////////////////////////
// Constants
////////////////////////////////////
#define DEFAULT_LINE_SEP "\n"
#define DEFAULT_LINE_BEG ""
#define DEFAULT_QUEUE_SIZE 1024U
#define DEFAULT_TIME_FORMAT "%Y-%m-%d %H:%M:%S"


////////////////////////////////////
// Types
////////////////////////////////////

// Messages are always created and must be freed
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
    uint32_t size;
    LogMessage** messages;
    MUTEX_TYPE queue_write_lock;
    MUTEX_TYPE queue_read_lock;
    COND_TYPE queue_not_full;
    COND_TYPE queue_not_empty;
} LogQueue;

// Streams, high level buffered io
typedef struct {
    FILE* handle;
    CloseStreamHandler close_handler;
} LogStreamHandler;

// Low level descriptors
typedef struct {
    int handle;
    CloseDescriptorHandler close_handler;
} LogDescriptorHandler;

typedef struct {
    UserLogFormat* format;
    LogDescriptorHandler* descriptor_handler;
    LogStreamHandler* stream_handler;
} LogFormat;


// Contains everything a logger will need
typedef struct {
    LogFormat* log_format;
    int32_t id;
    bool done;
    THREAD_TYPE thread_id;
    LogQueue* queue;
} Logger;


////////////////////////////////////
// Values
////////////////////////////////////
static Logger** loggers = 0;
static int32_t loggers_length = 0;
static int32_t total_loggers = 0;
static LogMessage TERMINATE = {.done = true};


////////////////////////////////////
// Declarations
////////////////////////////////////

// Queue
static LogQueue* CreateQueue(uint32_t capacity);
static void DestroyQueue(LogQueue** queue);
static bool IsQueueFull(LogQueue* queue);
static bool IsQueueEmpty(LogQueue* queue);
static bool Enqueue(LogQueue* queue, LogMessage* message);
static LogMessage* Dequeue(LogQueue* queue);

// Logging
static void* RunLogger(void* arg);
static void CleanUpLogger(int32_t id);
static const char* StringFromLevel(LogLevel level);
static const char* ColorFromLevel(LogLevel level);
static LogFormat* CreateLogFormat(UserLogFormat* user_format, LogStreamHandler* stream_handler, LogDescriptorHandler* descriptor_handler);
static void LogHandler(LogMessage* message, LogFormat* log_format);
static int32_t InitLogger(LogFormat* log_format);

/*
 * Create the log queue and allocate the full capacity up front
 */
static LogQueue* CreateQueue(uint32_t capacity) {
    #ifdef DEBUG
        assert(capacity > 0U);
    #endif
    LogQueue* queue = error_checked_malloc(sizeof(LogQueue));

    // Clear out values and set actual ones
    memset(queue, 0U, sizeof(*queue));
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->size = 0U;

    // Init locks/cond
    MUTEX_INIT(queue->queue_write_lock);
    MUTEX_INIT(queue->queue_read_lock);
    COND_INIT(queue->queue_not_full);
    COND_INIT(queue->queue_not_empty);

    // Init messsages circular buffer
    queue->messages = error_checked_malloc(sizeof(LogMessage*) * queue->capacity);
    memset(queue->messages, 0U, sizeof(LogMessage*) * queue->capacity);
    return queue;
}


/*
 * Take care in deleting all of the messages and freeing the actual message types.
 * The queue should be empty by this point but its good to iterate and check
 */
static void DestroyQueue(LogQueue** queue) {
    #ifdef DEBUG
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

    // Clean up threading stuff
    MUTEX_DESTROY(((*queue)->queue_write_lock));
     MUTEX_DESTROY(((*queue)->queue_read_lock));
    COND_DESTROY(((*queue)->queue_not_empty));
    COND_DESTROY(((*queue)->queue_not_full));

    // Free messages that the queue owns
    if ((*queue)->messages) {
        free((*queue)->messages);
        (*queue)->messages = NULL;
    }

    // Free the queue itself
    if (*queue) {
        (*queue)->capacity = (*queue)->head = (*queue)->tail = 0U;
        free(*queue);
        *queue = NULL;
    }
}


/* 
 * If head + 1 == tail
 */
static bool IsQueueFull(LogQueue* queue) {
    return queue->head + 1 == queue->tail;
}


/* 
 * If head is tail
 */
static bool IsQueueEmpty(LogQueue* queue) {
    return queue->head == queue->tail;
}


/* 
 * Lock the write mutex
 * Put message in queue if its not full
 * Signal a message is in the queue
 * Unlock mutex
 */
static bool Enqueue(LogQueue* queue, LogMessage* message) {
    MUTEX_LOCK(queue->queue_write_lock);

    #ifdef DEBUG
        assert(queue);
        assert(message);
    #endif
    
    // Just dont queue if full
    while (IsQueueFull(queue)) {
        COND_WAIT(queue->queue_not_full, queue->queue_write_lock);
    }
    
    // Add message and advance queue
    queue->messages[queue->head] = message;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size++;

    // Let the thread know it has a message
    COND_SIGNAL(queue->queue_not_empty);
    MUTEX_UNLOCK(queue->queue_write_lock);
    return true;
}


/* 
 * If queue is empty return nothing, otherwise return message
 */
static LogMessage* Dequeue(LogQueue* queue) {
    MUTEX_LOCK(queue->queue_read_lock);

    #ifdef DEBUG
        assert(queue);
    #endif

    while (IsQueueEmpty(queue)) {
        COND_WAIT(queue->queue_not_empty, queue->queue_read_lock);
    }

    LogMessage* message = queue->messages[queue->tail];
    queue->messages[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size--;

    COND_SIGNAL(queue->queue_not_full);
    MUTEX_UNLOCK(queue->queue_read_lock);
    return message;
    
}


/*
 * Grab messages in polling loop,
 * Then print them
 * Free messages after
 */
static void* RunLogger(void* arg) {
    #ifdef DEBUG
        assert(arg);
    #endif
    Logger* logger = arg;
    #ifdef DEBUG
        assert(logger->queue);
    #endif

    // For some reason you have to grab the read lock and read all that you can in a loop
    // Or else the condition is never signaled and you wait
    while (!logger->done) {
        LogMessage* message = Dequeue(logger->queue);

        #ifdef DEBUG
            assert(logger);
            assert(message);
        #endif

        if (message) {
            // Done at this point
            if (message->done) {
                logger->done = true;
                continue;
            }

            LogHandler(message, logger->log_format);
            
        }
    }

    DestroyQueue(&logger->queue);

    return EXIT_SUCCESS;
}


/*
 * Handle output to the handler and message clean up after
 */
static void LogHandler(LogMessage* message, LogFormat* log_format) {
    #ifdef DEBUG
        assert (message);
        assert(log_format);
        assert(log_format->format);
    #endif

    if (message->level >= log_format->format->level) {
        if(log_format->stream_handler) {
            if (log_format->format->colors) {
                STREAM_WRITE(ColorFromLevel(message->level), log_format->stream_handler->handle);
            }

            // Write actual output
            STREAM_WRITE(log_format->format->linebeg, log_format->stream_handler->handle);
            STREAM_WRITE(message->msg, log_format->stream_handler->handle);
            STREAM_WRITE(log_format->format->linesep, log_format->stream_handler->handle);

            // Reset colors now
            if (log_format->format->colors) {
                STREAM_WRITE(RESET, log_format->stream_handler->handle);
            }

            // Flush if needed
            if (log_format->format->flush) {
                fflush(log_format->stream_handler->handle);
            }
        } else if (log_format->descriptor_handler) {
            if (log_format->format->colors) {
                DESCRIPTOR_WRITE(ColorFromLevel(message->level), log_format->descriptor_handler->handle);
            }

            // Write actual output
            DESCRIPTOR_WRITE(log_format->format->linebeg, log_format->descriptor_handler->handle);
            DESCRIPTOR_WRITE(message->msg, log_format->descriptor_handler->handle);
            DESCRIPTOR_WRITE(log_format->format->linesep, log_format->descriptor_handler->handle);

            // Reset colors now
            if (log_format->format->colors) {
                DESCRIPTOR_WRITE(RESET, log_format->descriptor_handler->handle);
            }
        }
        
    }

    // Clean up message
    free(message->msg);
    message->msg = NULL;

    // Clean up message
    free(message);
}


/* 
 * Helper method to allocate the LogFormat
 */
static LogFormat* CreateLogFormat(UserLogFormat* user_format, LogStreamHandler* stream_handler, LogDescriptorHandler* descriptor_handler) {
    #ifdef DEBUG
        assert(user_format);
    #endif

    // Create User handler
    LogFormat* log_format = error_checked_malloc(sizeof(LogFormat));
    memset(log_format, 0, sizeof(*log_format));

    // Copy over user format
    log_format->format = error_checked_malloc(sizeof(UserLogFormat));
    memset(log_format->format, 0, sizeof(*(log_format->format)));

    if (user_format) { 
        memcpy(log_format->format, user_format, sizeof(*(log_format->format)));
    }

    // Set defaults
    // Level defaults to LOG_DEBUG
    // colors defaults to 0 (false)
    // flush defaults to 0 (false)

    if (!log_format->format->linesep) log_format->format->linesep = DEFAULT_LINE_SEP;
    if (log_format->format->max_queue_size == 0) log_format->format->max_queue_size = DEFAULT_QUEUE_SIZE;
    if (!log_format->format->time_format) log_format->format->time_format = DEFAULT_TIME_FORMAT;
    if (!log_format->format->linebeg) log_format->format->linebeg = DEFAULT_LINE_BEG;

    if (stream_handler) {
        log_format->stream_handler = stream_handler;
    } else if (descriptor_handler) {
        log_format->descriptor_handler = descriptor_handler;
    } 
    return log_format;    
}


int32_t InitStreamLogger(UserLogFormat* user_format, FILE* handle, CloseStreamHandler close_handler) {
    #ifdef DEBUG
        assert(user_format);
    #endif

    if (!handle) return -1;

    // Create Handler
    LogStreamHandler* handler = error_checked_malloc(sizeof(LogStreamHandler));
    memset(handler, 0, sizeof(*handler));
    handler->handle = handle;
    handler->close_handler = close_handler;
    if (!handler->close_handler) handler->close_handler = DEFAULT_FILE_CLOSE_HANDLER;

    LogFormat* format = CreateLogFormat(user_format, handler, NULL);
    return InitLogger(format);

}

int32_t InitDescriptorLogger(UserLogFormat* user_format, int fd, CloseDescriptorHandler close_handler) {
    #ifdef DEBUG
        assert(user_format);
    #endif

    if (fd < 0) return -1;

    // Create Handler
    LogDescriptorHandler* handler = error_checked_malloc(sizeof(LogDescriptorHandler));
    memset(handler, 0, sizeof(*handler));
    handler->handle = fd;
    handler->close_handler = close_handler;
    if (!handler->close_handler) handler->close_handler = DEFAULT_DESCRIPTOR_CLOSE_HANDLER;

    LogFormat* format = CreateLogFormat(user_format, NULL, handler);
    return InitLogger(format);
}


/* 
 * Setup logger list
 * Add more entries as needed
 * Start logger threads.
 */
static int32_t InitLogger(LogFormat* log_format) {
    #ifdef DEBUG
        assert(log_format);
    #endif
    Logger* logger = NULL;


    // Allocate logger memory
    if (!loggers) {
        loggers = error_checked_malloc(sizeof(Logger*));
        loggers[0] = error_checked_malloc(sizeof(Logger));
        memset(loggers[loggers_length], 0, sizeof(*loggers[loggers_length]));
        logger = loggers[loggers_length];
        logger->id = loggers_length;
        loggers_length++;
    } else {
        loggers = error_checked_realloc(loggers, sizeof(Logger*) * (loggers_length + 1));
        loggers[loggers_length] = NULL;
        loggers[loggers_length] = error_checked_malloc(sizeof(Logger));
        memset(loggers[loggers_length], 0, sizeof(*loggers[loggers_length]));
        logger = loggers[loggers_length];
        logger->id = loggers_length;
        loggers_length++;
    }

    // Add format/queue
    logger->log_format = log_format;
    logger->queue = CreateQueue(logger->log_format->format->max_queue_size);

    // One more! One more!
    total_loggers++;

    // Spin up a thread for the loggers
    THREAD_CREATE(&logger->thread_id, RunLogger, ((void*)logger));

    // Return Id to user
    return logger->id;
}


/* 
 * Clean up routing
 */
static void CleanUpLogger(int32_t id) {
    // Wait for messages to end by queueing up TERMINATE and
    // then joining thread
    Enqueue(loggers[id]->queue, &TERMINATE);
    THREAD_JOIN(loggers[id]->thread_id);
    loggers[id]->queue = NULL;

    // Close the handler specified by format
    if (loggers[id]->log_format->stream_handler) {
        loggers[id]->log_format->stream_handler->close_handler(loggers[id]->log_format->stream_handler->handle);
        free(loggers[id]->log_format->stream_handler);
    } else if (loggers[id]->log_format->descriptor_handler) {
        loggers[id]->log_format->descriptor_handler->close_handler(loggers[id]->log_format->descriptor_handler->handle);
        free(loggers[id]->log_format->descriptor_handler);
    }

    loggers[id]->log_format->descriptor_handler = NULL;
    loggers[id]->log_format->stream_handler = NULL;

    // Free format
    free(loggers[id]->log_format->format);
    loggers[id]->log_format->format = NULL;
    free(loggers[id]->log_format);
    loggers[id]->log_format = NULL;

    // Free logger
    free(loggers[id]);

    // Set to NULL
    memset(loggers[id], 0, sizeof(Logger));

    // Decrement total count
    total_loggers--;
}


/*
 * Close all loggers
 */
void ShutdownLoggers() {
    for (int32_t idx = 0; idx < loggers_length; idx++) {
        if (loggers[idx]) {
            CleanUpLogger(idx);
        }
    }

    // Deallocate memory
    memset(loggers, 0, loggers_length * sizeof(Logger*));
    free(loggers);
    loggers = NULL;
    total_loggers = 0;
    loggers_length = 0;
}


/*
 * Stringify Level
 */
const char* StringFromLevel(LogLevel level) {
    const char* result = NULL;
    switch(level) {
        case LOG_DEBUG:
            result = "DEBUG";
            break;
        case LOG_INFO:
            result = "INFO";
            break;
        case LOG_WARN:
            result = "WARN";
            break;
        case LOG_ERROR:
            result = "ERROR";
            break;
        case LOG_FATAL:
            result = "FATAL";
            break;
        default:
            result = "UNKNOWN";
            break;
    }

    return result;
}

/*
 * Colorfy Level
 */
const char* ColorFromLevel(LogLevel level) {
    const char* result = NULL;
    switch(level) {
        case LOG_DEBUG:
            result = CYAN;
            break;
        case LOG_INFO:
            result = GREEN;
            break;
        case LOG_WARN:
            result = YELLOW;
            break;
        case LOG_ERROR:
            result = RED;
            break;
        case LOG_FATAL:
            result = MAGENTA;
            break;
        default:
            result = WHITE;
            break;
    }

    return result;
}


/*
 * Log a single const string
 */
void Log(int32_t logger_id, LogLevel level, const char* msg) {
    if (logger_id >= loggers_length || logger_id < 0) {
        fprintf(stderr, "Invalid Logger Id %d\n", logger_id);
        ShutdownLoggers();
        exit(EXIT_FAILURE);
    }

    // Get associated logger from table
    Logger* logger = loggers[logger_id];

    if (!logger) {
        fprintf(stderr, "Invalid Logger...\n");
        ShutdownLoggers();
        exit(EXIT_FAILURE);
    }

    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;
    const char* level_string = StringFromLevel(level);
    uint32_t level_size = strlen(level_string);
    char buf[128];
    time_t current_time = time(NULL);
    struct tm* ct = localtime(&current_time);
    buf[strftime(buf, sizeof(buf), logger->log_format->format->time_format, ct)] = '\0';
    LogMessage* message = error_checked_malloc(sizeof(LogMessage));
    memset(message, 0, sizeof(*message));
    
    // Create message here
    formatted_msg = error_checked_malloc(strlen(msg) + strlen(buf) + level_size + padding + 1);
    sprintf(formatted_msg, "[%s] %s %s", buf, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    // Create message finally
    Enqueue(logger->queue, message);
}

/*
 * Log a single string then free it if freeit is true
 */
void Log2(int32_t logger_id, LogLevel level, char* msg, bool free_string) {
    if (logger_id >= loggers_length || logger_id < 0) {
        fprintf(stderr, "Invalid Logger Id %d\n", logger_id);
        ShutdownLoggers();
        exit(EXIT_FAILURE);
    }

    // Get associated logger from table
    Logger* logger = loggers[logger_id];

    if (!logger) {
        fprintf(stderr, "Invalid Logger...\n");
        ShutdownLoggers();
        exit(EXIT_FAILURE);
    }

    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;
    const char* level_string = StringFromLevel(level);
    uint32_t level_size = strlen(level_string);
    char buf[128];
    time_t current_time = time(NULL);
    struct tm* ct = localtime(&current_time);
    buf[strftime(buf, sizeof(buf), logger->log_format->format->time_format, ct)] = '\0';
    LogMessage* message = error_checked_malloc(sizeof(LogMessage));
    memset(message, 0, sizeof(*message));
    
    // Create message here
    formatted_msg = error_checked_malloc(strlen(msg) + strlen(buf) + level_size + padding + 1);
    sprintf(formatted_msg, "[%s] %s %s", buf, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    // Free original message if needed for the user
    if (free_string) {
        free(msg);
    }

    Enqueue(logger->queue, message);
}
