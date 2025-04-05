#pragma once


#include <stdarg.h>
#include <stdio.h>


// Structure to hold the formatted message and its size
typedef struct {
    char *message;  // The formatted message
    size_t size;    // The size of the formatted message
    int endWithNewLine;   // 1 if it ends with a new line
    unsigned int repeat; // How many times the message was repeated
} LogFormattedMessage;

// List of status
enum { LOG_TRACE, // 0
       LOG_DEBUG, // 1
       LOG_GAME,  // 2
       LOG_INFO,  // 3
       LOG_WARN,  // 4
       LOG_ERROR, // 5
       LOG_FATAL  // 6
};

// Macro to ease logging
#define log_game(...)  logGeneric(LOG_GAME,  __FILE__, __LINE__, __VA_ARGS__)
#define log_trace(...) logGeneric(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) logGeneric(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  logGeneric(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  logGeneric(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logGeneric(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) logGeneric(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

// Macro to ease logging
#define logVA_game(...)  logVA(LOG_GAME,  __FILE__, __LINE__, __VA_ARGS__)


int logGeneric(int level, const char *file, int line, const char *message, ...);
int logVA(int level, const char *file, int line, const char *message, va_list args);
int logSanityChecks(int level, const char *message);
int logPrintHeader(FILE *stream, int level);
int logPrintMessage(FILE *stream, LogFormattedMessage formattedMessage, int level);
LogFormattedMessage logFormatMessage(const char *message, va_list args);
FILE* logGetStream();
void logInitTimer();
void logGetElapsedTime(long *seconds, long *milliseconds);
