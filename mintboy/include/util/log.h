#ifndef LOGGO_H
#define LOGGO_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(LOGGO_USE_POSIX)
    #include <pthread.h>
    #include <unistd.h>
#elif defined(_WIN32) || defined(LOGGO_USE_WINDOWS)
    #include <io.h>
    #include <Windows.h>
#endif

#ifdef LOGGO_DEBUG
    #include <assert.h>
#endif

// Change this for shorter names
#ifndef LOGGO_SHORT_NAMES
    #define LOGGO_LONG_NAMES
#endif

#ifdef LOGGO_SHORT_NAMES
    #define LOGGO_NAME(name) name
#endif

#ifdef LOGGO_LONG_NAMES
    #define LOGGO_NAME(name) Loggo_ ## name
#endif

// Change this to change how Loggo is compiled in
// Static would obviously make scope Loggo API methods to the 
// current translation unit which could be what you want
// Defaults to extern API so define LOGGO_IMPLEMENTATION in the file you want
// to provide definitions in
#ifndef LOGGODEF
    #ifdef LOGGO_DEF_STATIC
        #define LOGGODEF static
    #else
        #define LOGGODEF extern
    #endif
#endif

// Log Levels
typedef enum {
    LOGGO_LEVEL_DEBUG,
    LOGGO_LEVEL_INFO,
    LOGGO_LEVEL_WARN,
    LOGGO_LEVEL_ERROR,
    LOGGO_LEVEL_FATAL
} LoggoLogLevel;

typedef int (*CloseHandler)(void*);
typedef int (*WriteHandler)(char*, void*);
typedef int (*FlushHandler)(void*);

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* handle;
    CloseHandler close_handler;
    WriteHandler write_handler;
    FlushHandler flush_handler;
} LoggoLogHandler;

// The user controls the format
typedef struct {
    LoggoLogLevel level;
    uint32_t queue_capacity;
    bool colors;
    bool flush;
    char* time_format;
    char* linesep;
    char* linebeg;
} LoggoLogFormat;


// API

/* 
 * Init Logger in its own thread.
 * If user_format is NULL then the defaults are used
 * If user_handler is NULL then buffered stdout is used for logging
 * name cannot be NULL
 * Returns logger id on success or -1 for Failure
 */
LOGGODEF int32_t LOGGO_NAME(CreateLogger)(const char* name, LoggoLogFormat* user_format, LoggoLogHandler* user_handler);


/* 
 * Delete logger waiting for all of its messages,
 * This will also clean up the resources if its the last logger so there is no need to call DeleteLoggers
 */
LOGGODEF void LOGGO_NAME(DeleteLogger)(const char* name);


/* 
 * Stop Logger threads and clean up handles.
 * This is idempotent so it can be called multiple times
 */
 LOGGODEF void LOGGO_NAME(DeleteLoggers)();


/* 
 * Pass messages to the log queue, the logging thread will accept messages,
 * then use the handler methods (or defaults) to output logs
 */
LOGGODEF void LOGGO_NAME(Log)(const char* name, LoggoLogLevel level, const char* msg);
LOGGODEF void LOGGO_NAME(Log2)(const char* name, LoggoLogLevel level, char* msg, bool free_string);

// Loggo Handler methods

// FILE* friends
LOGGODEF int LOGGO_NAME(StreamWrite)(char* text, void* arg);
LOGGODEF int LOGGO_NAME(StreamClose)(void* arg);
LOGGODEF int LOGGO_NAME(StreamFlush)(void* arg);

// Raw Descriptor IO
LOGGODEF int LOGGO_NAME(DescriptorWrite)(char* text, void* arg);
LOGGODEF int LOGGO_NAME(DescriptorClose)(void* arg);
LOGGODEF int LOGGO_NAME(DescriptorFlush)(void* arg);

// Do nothing
LOGGODEF int LOGGO_NAME(NullWrite)(char* text, void* arg);
LOGGODEF int LOGGO_NAME(NullClose)(void* arg);
LOGGODEF int LOGGO_NAME(NullFlush)(void* arg);

#ifdef __cplusplus
}
#endif

// Convenience Macros for logging
#ifdef LOGGO_USE_HELPERS
    #define LOG_DEBUG(name, msg) LOGGO_NAME(Log)((name), LOGGO_LEVEL_DEBUG, (msg))
    #define LOG_INFO(name, msg) LOGGO_NAME(Log)((name), LOGGO_LEVEL_INFO, (msg))
    #define LOG_WARN(name, msg) LOGGO_NAME(Log)((name), LOGGO_LEVEL_WARN, (msg))
    #define LOG_ERROR(name, msg) LOGGO_NAME(Log)((name), LOGGO_LEVEL_ERROR, (msg))
    #define LOG_FATAL(name, msg) LOGGO_NAME(Log)((name), LOGGO_LEVEL_FATAL, (msg))

    #define LOG2_DEBUG(name, msg, free_string) LOGGO_NAME(Log2)((name), LOGGO_LEVEL_DEBUG, (msg), (free_string))
    #define LOG2_INFO(name, msg, free_string) LOGGO_NAME(Log2)((name), LOGGO_LEVEL_INFO, (msg), (free_string))
    #define LOG2_WARN(name, msg, free_string) LOGGO_NAME(Log2)((name), LOGGO_LEVEL_WARN, (msg), (free_string))
    #define LOG2_ERROR(name, msg, free_string) LOGGO_NAME(Log2)((name), LOGGO_LEVEL_ERROR, (msg), (free_string))
    #define LOG2_FATAL(name, msg, free_string) LOGGO_NAME(Log2)((name), LOGGO_LEVEL_FATAL, (msg), (free_string))

    #define STDOUT_STREAM_HANDLER (LoggoLogHandler) { \
                                    .handle=stdout, \
                                    .write_handler=LOGGO_NAME(StreamWrite), \
                                    .close_handler=LOGGO_NAME(StreamClose), \
                                    .flush_handler=LOGGO_NAME(StreamFlush) \
                                }

    #define STDERR_STREAM_HANDLER (LoggoLogHandler) { \
                                    .handle=stderr, \
                                    .write_handler=LOGGO_NAME(StreamWrite), \
                                    .close_handler=LOGGO_NAME(StreamClose), \
                                    .flush_handler=LOGGO_NAME(StreamFlush) \
                                }

    #define STDOUT_DESC_HANDLER (LoggoLogHandler) { \
                                    .handle=(&STDOUT_FILENO), \
                                    .write_handler=LOGGO_NAME(DescriptorWrite), \
                                    .close_handler=LOGGO_NAME(DescriptorClose), \
                                    .flush_handler=LOGGO_NAME(DescriptroFlush) \
                                }

    #define STDERR_DESC_HANDLER (LoggoLogHandler) { \
                                .handle=(&STDERR_FILENO), \
                                .write_handler=LOGGO_NAME(DescriptorWrite), \
                                .close_handler=LOGGO_NAME(DescriptorClose), \
                                .flush_handler=LOGGO_NAME(DescriptroFlush) \
                            }
#endif

// Do implementation here if you do this twice when LOGGO_DEF_STATIC is not set you will get linker
// errors from multiple definitions
#ifdef LOGGO_IMPLEMENTATION

////////////////////////////////////
// Platform and Helpers
////////////////////////////////////

#define LOGGO_UNUSED(x) (void)(x)

#if defined(__unix__) || defined(linux) || defined(__APPLE__) || defined(LOGGO_USE_POSIX)
    #define LOGGO_THREAD_TYPE pthread_t
    #define LOGGO_THREAD_CREATE(id, func, param) pthread_create((id), NULL, (func), (param))
    #define LOGGO_THREAD_JOIN(id) pthread_join((id), (NULL))
    #define LOGGO_MUTEX_TYPE pthread_mutex_t
    #define LOGGO_MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
    #define LOGGO_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
    #define LOGGO_MUTEX_LOCK(mutex) pthread_mutex_lock(&(mutex))
    #define LOGGO_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&(mutex))
    #define LOGGO_COND_TYPE pthread_cond_t
    #define LOGGO_COND_INIT(condition) pthread_cond_init(&(condition), NULL)
    #define LOGGO_COND_DESTROY(condition) pthread_cond_destroy(&(condition))
    #define LOGGO_COND_WAIT(condition, mutex) pthread_cond_wait(&(condition), &(mutex))
    #define LOGGO_COND_SIGNAL(condition) pthread_cond_signal(&(condition))
    #define LOGGO_RED      "\x1B[31m"
    #define LOGGO_GREEN    "\x1B[32m"
    #define LOGGO_YELLOW   "\x1B[33m"
    #define LOGGO_BLUE     "\x1B[34m"
    #define LOGGO_MAGENTA  "\x1B[35m"
    #define LOGGO_CYAN     "\x1B[36m"
    #define LOGGO_WHITE    "\x1B[37m"
    #define LOGGO_RESET    "\033[0m"

    LOGGODEF int LOGGO_NAME(DescriptorWrite)(char* text, void* arg) {
        return write(*(int*)arg, text, strlen(text));
    }


    LOGGODEF int LOGGO_NAME(DescriptorClose)(void* arg) {
        return close(*(int*)arg);
    }
#elif defined(_WIN32) || defined(LOGGO_USE_WINDOWS)
    #define LOGGO_THREAD_TYPE LPDWORD
    #define LOGGO_THREAD_CREATE(id, func, param) CreateThread(NULL, 0, func, param, 0, id)
    #define LOGGO_THREAD_JOIN(id) WaitForSingleObject((id), INFINITE)
    #define LOGGO_MUTEX_TYPE LPCRITICAL_SECTION
    #define LOGGO_MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
    #define LOGGO_MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
    #define LOGGO_MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
    #define LOGGO_MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
    #define LOGGO_COND_TYPE PCONDITION_VARIABLE
    #define LOGGO_COND_INIT(condition) InitializeConditionVariable((condition))
    #define LOGGO_COND_DESTROY(condition) DeleteConditionVariable((condition))
    #define LOGGO_COND_WAIT(condition, mutex) SleepConditionVariableCS((condition), (mutex), INFINITE)
    #define LOGGO_COND_SIGNAL(condition) WakeConditionVariable((condition))
    // TODO Fix this
    #define LOGGO_RED      ""
    #define LOGGO_GREEN    ""
    #define LOGGO_YELLOW   ""
    #define LOGGO_BLUE     ""
    #define LOGGO_MAGENTA  ""
    #define LOGGO_CYAN     ""
    #define LOGGO_WHITE    ""
    #define LOGGO_RESET    ""

    LOGGODEF int LOGGO_NAME(DescriptorWrite)(char* text, void* arg) {
        return _write(*(int*)arg, text, strlen(text));
    }

    
    LOGGODEF int LOGGO_NAME(DescriptorClose)(void* arg) {
        return _close(*(int*)arg);
    }
#endif

// Common

LOGGODEF int LOGGO_NAME(DescriptorFlush)(void* arg) {
    LOGGO_UNUSED(arg);
    return 0;
}


LOGGODEF int LOGGO_NAME(StreamWrite)(char* text, void* arg) {
    return fputs(text, (FILE*)arg);
}


LOGGODEF int LOGGO_NAME(StreamClose)(void* arg) {
   return fclose((FILE*)arg);
}


LOGGODEF int LOGGO_NAME(StreamFlush)(void* arg) {
    return fflush((FILE*)arg);
}


LOGGODEF int LOGGO_NAME(NullWrite)(char* text, void* arg) {
    LOGGO_UNUSED(arg);
    LOGGO_UNUSED(text);
    return 0;
}


LOGGODEF int LOGGO_NAME(NullClose)(void* arg) {
    LOGGO_UNUSED(arg);
    return 0;
}


LOGGODEF int LOGGO_NAME(NullFlush)(void* arg) {
    LOGGO_UNUSED(arg);
    return 0;
}



static void* LOGGO_NAME(ErrorCheckedMalloc)(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because malloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


static void* LOGGO_NAME(ErrorCheckedRealloc)(void* original, size_t size) {
    void* ptr = realloc(original, size);
    if (!ptr) {
        fprintf(stderr, "[ERROR] Exiting because realloc failed...");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


////////////////////////////////////
// Defaults
////////////////////////////////////

#define LOGGO_DEFAULT_LINE_SEP "\n"
#define LOGGO_DEFAULT_LINE_BEG ""
#define LOGGO_DEFAULT_QUEUE_SIZE 1024U 
#define LOGGO_DEFAULT_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define LOGGO_DEFAULT_HT_INITIAL_CAPACITY 128
#define LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR 0.7f

// Can be overriden by user
#define LOGGO_MALLOC LOGGO_NAME(ErrorCheckedMalloc)
#define LOGGO_REALLOC LOGGO_NAME(ErrorCheckedRealloc)
#define LOGGO_FREE free


////////////////////////////////////
// Types
////////////////////////////////////

// Messages are always created and must be freed
typedef struct {
    LoggoLogLevel level;
    bool done;
    char* msg;
} LoggoLogMessage;



// Circular dynamic array implementation
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t capacity;
    uint32_t size;
    LoggoLogMessage** messages;
    LOGGO_MUTEX_TYPE queue_lock;
    LOGGO_COND_TYPE queue_not_full;
    LOGGO_COND_TYPE queue_not_empty;
} LoggoLogQueue;


// Contains everything a logger will need
typedef struct {
    LoggoLogFormat* format;
    LoggoLogHandler* handler;
    LoggoLogQueue* queue;
    int32_t id;
    LOGGO_THREAD_TYPE thread_id;
    const char* name;
    bool done;

} LoggoLogger;

typedef struct {
    LoggoLogger** loggers;
    int32_t size;
    int32_t capacity;
    double load_factor;
} LoggoHashTable;


////////////////////////////////////
// Constants
////////////////////////////////////
static const int32_t LOGGO_PRIME_1 = 71U;
static const int32_t LOGGO_PRIME_2 = 197U;


////////////////////////////////////
// Global values
////////////////////////////////////
static LoggoLogMessage LOGGO_LOGGER_TERMINATE = {.done = true};
static LoggoLogger LOGGO_LOGGER_DELETED = {0};
static LoggoHashTable LOGGO_LOGGER_HASH_TABLE = {0};


////////////////////////////////////
// Declarations up front (not including api)
////////////////////////////////////


// Queue
static LoggoLogQueue* LOGGO_NAME(CreateQueue)(uint32_t capacity);
static void LOGGO_NAME(DestroyQueue)(LoggoLogQueue* queue);
static bool LOGGO_NAME(IsQueueFull)(LoggoLogQueue* queue);
static bool LOGGO_NAME(IsQueueEmpty)(LoggoLogQueue* queue);
static void LOGGO_NAME(Enqueue)(LoggoLogQueue* queue, LoggoLogMessage* message);
static LoggoLogMessage* LOGGO_NAME(Dequeue)(LoggoLogQueue* queue);

// Logging
static void* LOGGO_NAME(RunLogger)(void* arg);
static char* LOGGO_NAME(StringFromLevel)(LoggoLogLevel level);
static char* LOGGO_NAME(ColorFromLevel)(LoggoLogLevel level);
static LoggoLogMessage* LOGGO_NAME(CreateLogMessage)(LoggoLogger* logger, LoggoLogLevel level, const char* msg);
static LoggoLogFormat* LOGGO_NAME(CreateLogFormat)(LoggoLogFormat* user_format);
static LoggoLogHandler* LOGGO_NAME(CreateLogHandler)(LoggoLogHandler* user_handler);
static void LOGGO_NAME(DestroyLogHandler)(LoggoLogHandler* handler);
static void LOGGO_NAME(DestroyLogFormat)(LoggoLogFormat* format);
static void LOGGO_NAME(CleanUpLogger)(LoggoLogger* logger);
static void LOGGO_NAME(HandleLogMessage)(LoggoLogMessage* message, LoggoLogFormat* format, LoggoLogHandler* handler);

// Hash Table
static LoggoLogger* LOGGO_NAME(HTFindItem)(const char* name);
static int32_t LOGGO_NAME(HTInsertItem)(const char* name, LoggoLogger* logger);
static void LOGGO_NAME(HTDeleteItem)(const char* name);
static void LOGGO_NAME(HTResizeTable)();
static void LOGGO_NAME(HTInitTable)();
static void LOGGO_NAME(HTDeleteTable)();
static int32_t LOGGO_NAME(StringHash)(const char* name, const int32_t prime, const int32_t buckets);
static int32_t LOGGO_NAME(LoggerStringHash)(const char* name, const int32_t buckets, const int32_t attempt);



////////////////////////////////////
// Api
////////////////////////////////////


// Create logger components, start thread with the created queue
// Return -1 on error dont allocate anything, clean slate
LOGGODEF int32_t LOGGO_NAME(CreateLogger)(const char* name, LoggoLogFormat* user_format, LoggoLogHandler* user_handler) {
    #ifdef LOGGO_DEBUG
        assert(name);
    #endif

    if (!name) {
        return -1;
    }

    LOGGO_NAME(HTInitTable)();

    LoggoLogger* logger = LOGGO_MALLOC(sizeof(LoggoLogger));
    memset(logger, 0U, sizeof(*logger));

    // Fill up with info
    logger->handler = LOGGO_NAME(CreateLogHandler)(user_handler);

    // Clean up and return -1
    if (!logger->handler) {
        LOGGO_NAME(DestroyLogHandler)(logger->handler);
        memset(logger, 0U, sizeof(*logger));
        LOGGO_FREE(logger);
        logger = NULL;
        return  -1;
    }

    logger->format = LOGGO_NAME(CreateLogFormat)(user_format);
    logger->name = name;
    logger->queue = LOGGO_NAME(CreateQueue)(logger->format->queue_capacity);

    // Handle the string allocation to a logger id
    int32_t id = LOGGO_NAME(HTInsertItem(name, logger));

    // We failed
    if (id == -1) {
        LOGGO_NAME(DestroyLogFormat)(logger->format);
        LOGGO_NAME(DestroyLogHandler)(logger->handler);
        LOGGO_NAME(DestroyQueue)(logger->queue);
        memset(logger, 0U, sizeof(*logger));
        LOGGO_FREE(logger);
        logger = NULL;
        return  id;
    }

    // Handle new stuff
    logger->id = id;

    // Spin up a thread for the loggers
    LOGGO_THREAD_CREATE(&logger->thread_id, LOGGO_NAME(RunLogger), ((void*)logger));

    // Return Id to user
    return logger->id;
}


// Shutdown the loggers by iterating and setting values
LOGGODEF void LOGGO_NAME(DeleteLoggers)() {
    for (int32_t idx = 0; idx < LOGGO_LOGGER_HASH_TABLE.capacity; idx++) {
        // Delete non null items by grabbing there names from the table
        if (LOGGO_LOGGER_HASH_TABLE.loggers[idx] != NULL && LOGGO_LOGGER_HASH_TABLE.loggers[idx] != &LOGGO_LOGGER_DELETED) {
            LOGGO_NAME(HTDeleteItem)(LOGGO_LOGGER_HASH_TABLE.loggers[idx]->name);
        }
    }

    LOGGO_NAME(HTDeleteTable)();
    memset(&LOGGO_LOGGER_HASH_TABLE, 0U, sizeof(LoggoHashTable));
}

// Shutdown the loggers by iterating and setting values
LOGGODEF void LOGGO_NAME(DeleteLogger)(const char* name) {
    LOGGO_NAME(HTDeleteItem)(name);

    // Just delete the table and clear it so its inited next time
    if (LOGGO_LOGGER_HASH_TABLE.size == 0) {
        LOGGO_NAME(HTDeleteTable)();
        memset(&LOGGO_LOGGER_HASH_TABLE, 0U, sizeof(LoggoHashTable));
    }
}



// Log message with Enqueue
LOGGODEF void LOGGO_NAME(Log)(const char* name, LoggoLogLevel level, const char* msg) {
    #ifdef LOGGO_DEBUG
        assert(name);
        assert(msg);
        assert(level >= 0U);
    #endif

    LoggoLogger* logger = LOGGO_NAME(HTFindItem)(name);

    if (!logger) {
        fprintf(stderr, "Invalid Logger Name: %s\n", name);
        LOGGO_NAME(DeleteLoggers)();
        exit(EXIT_FAILURE);
    }

    LoggoLogMessage* message = LOGGO_NAME(CreateLogMessage)(logger, level, msg);


    LOGGO_NAME(Enqueue)(logger->queue, message);
}


// Log message with Enqueue, optionally free msg if free_string is true. In case the user wants to pass
// a malloced string in
LOGGODEF void LOGGO_NAME(Log2)(const char* name, LoggoLogLevel level, char* msg, bool free_string) {
    #ifdef LOGGO_DEBUG
        assert(name);
        assert(msg);
        assert(level >= 0U);
    #endif

    LoggoLogger* logger = LOGGO_NAME(HTFindItem)(name);

    if (!logger) {
        fprintf(stderr, "Invalid Logger Name: %s\n", name);
        LOGGO_NAME(DeleteLoggers)();
        exit(EXIT_FAILURE);
    }

    LoggoLogMessage* message = LOGGO_NAME(CreateLogMessage)(logger, level, msg);

    if (free_string) {
        free(msg);
    }
    
    LOGGO_NAME(Enqueue)(logger->queue, message);
}


// Queue

// Create the queue with sane defaults
static LoggoLogQueue* LOGGO_NAME(CreateQueue)(uint32_t capacity) {
    #ifdef LOGGO_DEBUG
        assert(capacity > 0U);
    #endif

    LoggoLogQueue* queue = LOGGO_MALLOC(sizeof(LoggoLogQueue));

    // Clear out values and set actual ones
    memset(queue, 0U, sizeof(*queue));
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->size = 0U;

    // Init locks/cond
    LOGGO_MUTEX_INIT(queue->queue_lock);
    LOGGO_COND_INIT(queue->queue_not_full);
    LOGGO_COND_INIT(queue->queue_not_empty);

    // Init messsages circular buffer
    queue->messages = LOGGO_MALLOC(sizeof(LoggoLogMessage*) * queue->capacity);
    memset(queue->messages, 0U, sizeof(LoggoLogMessage*) * queue->capacity);
    return queue;
}


// Free messages, queue and zero out mem
static void LOGGO_NAME(DestroyQueue)(LoggoLogQueue* queue) {
    #ifdef LOGGO_DEBUG
        assert(queue);
    #endif

    // Safer to go over all of them just in case and free shit,
    // The terminate in the thread loop should do this
    uint32_t start = queue->tail;
    uint32_t end = queue->head;
    while(start != end) {
        if(queue->messages[start]) {
            if (queue->messages[start]->msg) {
                LOGGO_FREE(queue->messages[start]->msg);
                queue->messages[start]->msg = NULL;
            }
            LOGGO_FREE(queue->messages[start]);
            queue->messages[start] = NULL;
        }
        
        // Wraparound
        start = (start + 1) % queue->capacity;
    }

    // Clean up threading stuff
    LOGGO_MUTEX_DESTROY((queue->queue_lock));
    LOGGO_COND_DESTROY((queue->queue_not_empty));
    LOGGO_COND_DESTROY((queue->queue_not_full));

    // Free messages that the queue owns
    if (queue->messages) {
        LOGGO_FREE(queue->messages);
        queue->messages = NULL;
    }

    // Clean up last bits of memory
    memset(queue, 0U, sizeof(*queue));
    LOGGO_FREE(queue);
}


// If head + 1 == tail
static bool LOGGO_NAME(IsQueueFull)(LoggoLogQueue* queue) {
    return queue->head + 1 == queue->tail;
}

 
// If head == tail
static bool LOGGO_NAME(IsQueueEmpty)(LoggoLogQueue* queue) {
    return queue->head == queue->tail;
}


// Wait for the queue to not be full
// Add message
// Signal that its not empty anymore
static void LOGGO_NAME(Enqueue)(LoggoLogQueue* queue, LoggoLogMessage* message) {
    LOGGO_MUTEX_LOCK(queue->queue_lock);

    #ifdef LOGGO_DEBUG
        assert(queue);
        assert(message);
    #endif
    
    // Just dont queue if full
    while (LOGGO_NAME(IsQueueFull)(queue)) {
        LOGGO_COND_WAIT(queue->queue_not_full, queue->queue_lock);
    }
    
    // Add message and advance queue
    queue->messages[queue->head] = message;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size++;

    // Let the thread know it has a message
    LOGGO_COND_SIGNAL(queue->queue_not_empty);
    LOGGO_MUTEX_UNLOCK(queue->queue_lock);
}


// If the queue is empty just wait until we get the okay from Enqueue
// Also let enqueue know we are not full because we took a message
static LoggoLogMessage* LOGGO_NAME(Dequeue)(LoggoLogQueue* queue) {
    LOGGO_MUTEX_LOCK(queue->queue_lock);

    #ifdef LOGGO_DEBUG
        assert(queue);
    #endif

    while (LOGGO_NAME(IsQueueEmpty)(queue)) {
        LOGGO_COND_WAIT(queue->queue_not_empty, queue->queue_lock);
    }

    LoggoLogMessage* message = queue->messages[queue->tail];
    queue->messages[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size--;

    LOGGO_COND_SIGNAL(queue->queue_not_full);
    LOGGO_MUTEX_UNLOCK(queue->queue_lock);
    return message; 
}


// Logging


// Stringify Level
static char* LOGGO_NAME(StringFromLevel)(LoggoLogLevel level) {
    char* result = NULL;
    switch(level) {
        case LOGGO_LEVEL_DEBUG:
            result = "DEBUG";
            break;
        case LOGGO_LEVEL_INFO:
            result = "INFO";
            break;
        case LOGGO_LEVEL_WARN:
            result = "WARN";
            break;
        case LOGGO_LEVEL_ERROR:
            result = "ERROR";
            break;
        case LOGGO_LEVEL_FATAL:
            result = "FATAL";
            break;
        default:
            result = "UNKNOWN";
            break;
    }

    return result;
}


// Colorify Level
static char* LOGGO_NAME(ColorFromLevel)(LoggoLogLevel level) {
    char* result = NULL;
    switch(level) {
        case LOGGO_LEVEL_DEBUG:
            result = LOGGO_GREEN;
            break;
        case LOGGO_LEVEL_INFO:
            result = LOGGO_CYAN;
            break;
        case LOGGO_LEVEL_WARN:
            result = LOGGO_YELLOW;
            break;
        case LOGGO_LEVEL_ERROR:
            result = LOGGO_RED;
            break;
        case LOGGO_LEVEL_FATAL:
            result = LOGGO_MAGENTA;
            break;
        default:
            result = LOGGO_WHITE;
            break;
    }

    return result;
}


static LoggoLogFormat* LOGGO_NAME(CreateLogFormat)(LoggoLogFormat* user_format) {
    #ifdef LOGGO_DEBUG
        assert(user_format);
    #endif

    // Create User handler
    LoggoLogFormat* log_format = LOGGO_MALLOC(sizeof(LoggoLogFormat));
    memset(log_format, 0U, sizeof(*log_format));

    // Copy user format if it exists
    if (user_format) { 
        memcpy(log_format, user_format, sizeof(*user_format));
    }

    // Set defaults
    // Level defaults to LOG_DEBUG
    // colors defaults to 0 (false)
    // flush defaults to 0 (false)

    if (!log_format->linesep) log_format->linesep = LOGGO_DEFAULT_LINE_SEP;
    if (log_format->queue_capacity == 0) log_format->queue_capacity = LOGGO_DEFAULT_QUEUE_SIZE;
    if (!log_format->time_format) log_format->time_format = LOGGO_DEFAULT_TIME_FORMAT;
    if (!log_format->linebeg) log_format->linebeg = LOGGO_DEFAULT_LINE_BEG;
    return log_format;
}


static LoggoLogHandler* LOGGO_NAME(CreateLogHandler)(LoggoLogHandler* user_handler) {
    #ifdef LOGGO_DEBUG
        assert(user_handler);
    #endif

    // Create User handler
    LoggoLogHandler* log_handler = LOGGO_MALLOC(sizeof(LoggoLogHandler));
    memset(log_handler, 0U, sizeof(*log_handler));

    // Copy user format if it exists
    if (user_handler) { 
        memcpy(log_handler, user_handler, sizeof(*user_handler));

        // You need the handle
        if (!log_handler->handle) {
            DestroyLogHandler(log_handler);
            return NULL;
        }

        // Set NULL handlers if they are not there
        if(!log_handler->write_handler) log_handler->write_handler = LOGGO_NAME(NullWrite);
        if(!log_handler->flush_handler) log_handler->flush_handler = LOGGO_NAME(NullFlush);
        if(!log_handler->close_handler) log_handler->close_handler = LOGGO_NAME(NullClose);

    } else {
        // Defaults
        log_handler->handle = stdout;
        log_handler->write_handler = LOGGO_NAME(StreamWrite);
        log_handler->close_handler = LOGGO_NAME(StreamClose);
        log_handler->flush_handler = LOGGO_NAME(StreamFlush);
    }

    return log_handler;  
}


static void LOGGO_NAME(DestroyLogHandler)(LoggoLogHandler* handler) {
    #ifdef LOGGO_DEBUG
        assert(handler);
    #endif

    memset(handler, 0U, sizeof(*handler));
    LOGGO_FREE(handler);  
}


static void LOGGO_NAME(DestroyLogFormat)(LoggoLogFormat* format) {
    #ifdef LOGGO_DEBUG
        assert(format);
    #endif

    memset(format, 0U, sizeof(*format));
    LOGGO_FREE(format);
}


// Actual ouptut of message and cleanup
static void LOGGO_NAME(HandleLogMessage)(LoggoLogMessage* message, LoggoLogFormat* format, LoggoLogHandler* handler) {
    #ifdef LOGGO_DEBUG
        assert(message);
        assert(format);
        assert(handler);
    #endif

    if (message->level >= format->level) {
        if (format->colors) {
            handler->write_handler(LOGGO_NAME(ColorFromLevel)(message->level), handler->handle);
        }

        // Write actual output
        handler->write_handler(format->linebeg, handler->handle);
        handler->write_handler(message->msg, handler->handle);
        handler->write_handler(format->linesep, handler->handle);

        // Reset colors
        if (format->colors) {
            handler->write_handler(LOGGO_RESET, handler->handle);
        }

        // Flush if needed
        if (format->flush) {
            handler->flush_handler(handler->handle);
        }
    }

    // Clean up message
    LOGGO_FREE(message->msg);
    message->msg = NULL;

    // Clean up message
    LOGGO_FREE(message);
}


// Thread spawned handler of messages
static void* LOGGO_NAME(RunLogger)(void* arg) {
    #ifdef LOGGO_DEBUG
        assert(arg);
    #endif
    LoggoLogger* logger = arg;
    #ifdef LOGGO_DEBUG
        assert(logger->queue);
    #endif

    // For some reason you have to grab the read lock and read all that you can in a loop
    // Or else the condition is never signaled and you wait
    while (!logger->done) {
        LoggoLogMessage* message = LOGGO_NAME(Dequeue)(logger->queue);

        #ifdef LOGGO_DEBUG
            assert(logger);
            assert(message);
        #endif

        if (message) {
            // Done at this point
            if (message->done) {
                logger->done = true;
                continue;
            }

            // Log the messages, then free them
            LOGGO_NAME(HandleLogMessage)(message, logger->format, logger->handler);
        }
    }

    return EXIT_SUCCESS;
}


// Create a nice formatted log message
static LoggoLogMessage* LOGGO_NAME(CreateLogMessage)(LoggoLogger* logger, LoggoLogLevel level, const char* msg) {
    // Misc
    char* formatted_msg = NULL;
    uint32_t padding = 3U + 2U;

    // Level String
    const char* level_string = LOGGO_NAME(StringFromLevel)(level);
    uint32_t level_size = strlen(level_string);

    // Get time
    char time_buffer[128U];
    time_t current_time = time(NULL);
    time_buffer[strftime(time_buffer, sizeof(time_buffer), logger->format->time_format, localtime(&current_time))] = '\0';

    // Create LogMessage
    LoggoLogMessage* message = LOGGO_MALLOC(sizeof(LoggoLogMessage));
    memset(message, 0U, sizeof(*message));
    
    // Insert formatted message inside of LogMessage
    formatted_msg = LOGGO_MALLOC(strlen(msg) + strlen(time_buffer) + level_size + padding + 1U);
    sprintf(formatted_msg, "[%s] %s %s", time_buffer, level_string, msg);
    message->level = level;
    message->msg = formatted_msg;

    return message;
}


// Free all the handles
static void LOGGO_NAME(CleanUpLogger)(LoggoLogger* logger) {

    // Queue up final message and wait for logger to close
    LOGGO_NAME(Enqueue)(logger->queue, &LOGGO_LOGGER_TERMINATE);
    LOGGO_THREAD_JOIN(logger->thread_id);

    // Free handles
    LOGGO_NAME(DestroyLogHandler)(logger->handler);
    logger->handler = NULL;

    LOGGO_NAME(DestroyLogFormat)(logger->format);
    logger->format = NULL;

    LOGGO_NAME(DestroyQueue)(logger->queue);
    logger->queue = NULL;

    LOGGO_FREE(logger);
}


// Logger hash table


// Try to find an item returning NULL if not found
static LoggoLogger* LOGGO_NAME(HTFindItem)(const char* name) {
    int32_t hash = LOGGO_NAME(LoggerStringHash)(name, LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    int32_t attempt = 1;
    LoggoLogger* current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    while(current_logger != NULL) {
        if (current_logger != &LOGGO_LOGGER_DELETED) {
            if (strcmp(name, current_logger->name) == 0) {
                return current_logger;
            }
        }
        hash = LOGGO_NAME(LoggerStringHash)(name, LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        attempt++;
    }

    return NULL;
}


// Try to insert an item, resize if needed
static int32_t LOGGO_NAME(HTInsertItem)(const char* name, LoggoLogger* logger) {
    // Try a resize
    LOGGO_NAME(HTResizeTable)();
    int32_t hash = LOGGO_NAME(LoggerStringHash)(name, LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    int32_t attempt = 1;
    LoggoLogger* current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    while(current_logger != NULL && current_logger != &LOGGO_LOGGER_DELETED) {
        hash = LOGGO_NAME(LoggerStringHash)(name, LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        // If the item exists update it
        if (current_logger) {
            if (strcmp(name, current_logger->name) == 0) {
                LOGGO_NAME(CleanUpLogger)(current_logger);
                LOGGO_LOGGER_HASH_TABLE.loggers[hash] = logger;
                return logger->id;
            }
        }

        attempt++;

    }

    LOGGO_LOGGER_HASH_TABLE.loggers[hash] = logger;
    LOGGO_LOGGER_HASH_TABLE.size++;
    logger->id = hash;
    return logger->id;
}


static void LOGGO_NAME(HTDeleteItem)( const char* name) {
    int32_t hash = LOGGO_NAME(LoggerStringHash)(name, LOGGO_LOGGER_HASH_TABLE.capacity, 0);
    LoggoLogger* current_logger = LOGGO_LOGGER_HASH_TABLE.loggers[hash];
    int32_t attempt = 1;
    while(current_logger != NULL) {
        if (current_logger != &LOGGO_LOGGER_DELETED && current_logger != NULL) {
            if (strcmp(current_logger->name, name) == 0) {
                LOGGO_NAME(CleanUpLogger)(current_logger);
                LOGGO_LOGGER_HASH_TABLE.loggers[hash] = &LOGGO_LOGGER_DELETED;
            }
        }
        hash = LOGGO_NAME(LoggerStringHash)(name, LOGGO_LOGGER_HASH_TABLE.capacity, attempt);
        current_logger =  LOGGO_LOGGER_HASH_TABLE.loggers[hash];
        attempt++;
    }
    LOGGO_LOGGER_HASH_TABLE.size--; 
}


// Just realloc space up
static void LOGGO_NAME(HTResizeTable)() {
    int32_t load = (LOGGO_LOGGER_HASH_TABLE.size * 100) / LOGGO_LOGGER_HASH_TABLE.capacity;
    if (load > (LOGGO_LOGGER_HASH_TABLE.load_factor * 100)) {
        LOGGO_LOGGER_HASH_TABLE.capacity *= 2;

        // No need to realloc a new table
        // Just realloc memory chunk
        LOGGO_LOGGER_HASH_TABLE.loggers = LOGGO_REALLOC(LOGGO_LOGGER_HASH_TABLE.loggers, sizeof(LoggoLogger*) * LOGGO_LOGGER_HASH_TABLE.capacity);
    }
}


// Delete table and set defaults
static void LOGGO_NAME(HTDeleteTable)() {
    if (LOGGO_LOGGER_HASH_TABLE.loggers) {
        LOGGO_FREE(LOGGO_LOGGER_HASH_TABLE.loggers);
        LOGGO_LOGGER_HASH_TABLE.loggers = NULL;
    }

    LOGGO_LOGGER_HASH_TABLE.capacity = LOGGO_DEFAULT_HT_INITIAL_CAPACITY;
    LOGGO_LOGGER_HASH_TABLE.size = 0;
    LOGGO_LOGGER_HASH_TABLE.load_factor = LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR;
}


// Init whats needed
static void LOGGO_NAME(HTInitTable)() {
    if (!LOGGO_LOGGER_HASH_TABLE.loggers) {
        LOGGO_LOGGER_HASH_TABLE.capacity = LOGGO_DEFAULT_HT_INITIAL_CAPACITY;
        LOGGO_LOGGER_HASH_TABLE.size = 0;
        LOGGO_LOGGER_HASH_TABLE.load_factor = LOGGO_DEFAULT_HT_INITIAL_LOAD_FACTOR;
        LOGGO_LOGGER_HASH_TABLE.loggers = LOGGO_MALLOC(sizeof(LoggoLogger*) * LOGGO_LOGGER_HASH_TABLE.capacity);
    }
}


// Simple hash
static int32_t LOGGO_NAME(StringHash)(const char* name, const int32_t prime, const int32_t buckets) {
    int32_t hash = 0;
    const int len = strlen(name);
    for (int32_t i = 0; i < len; i++) {
        hash += (int32_t)pow(prime, len - (i+1)) * name[i];
        hash = hash % buckets;
    }
    return hash; 
}


// Double hash
static int32_t LOGGO_NAME(LoggerStringHash)(const char* name, const int32_t buckets, const int32_t attempt) {
    const int32_t hasha = LOGGO_NAME(StringHash)(name, LOGGO_PRIME_1, buckets);
    const int32_t hashb = LOGGO_NAME(StringHash)(name, LOGGO_PRIME_2, buckets);
    return (hasha + (attempt * (hashb + 1))) % buckets;
}


#endif // LOGGO_IMPLEMENTATION

#endif // LOGGO_H
