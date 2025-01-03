#include "log.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>

// Control look (1 = enabled, else 0)
#define LOG_COLOR_ENABLED 1
#define LOG_TIME_ENABLED 1

// Minimum level to output logs, may be nice to replace it with a list in the future
#define LOG_MIN_LEVEL 2

// When enabled (1), it counts repeat message instead of spamming
#define LOG_NO_REPEAT 1
// Buffer size to check if a message is repeated. Too small will miss repeats, but too big take few bytes of memory
#define LOG_REPEAT_BUFFER_SIZE 8192


// Global variable to store the start time of the program
static struct timespec logStartTime = {0, 0};

// Hold a counter of message repeat
static unsigned int logRepeatCount  = 0;
static char logLastMessage[LOG_REPEAT_BUFFER_SIZE] = {0};

static const char *log_style[] = {
        "\x1b[0m",   // TRACE : Normal default color
        "\x1b[36m",  // DEBUG : Cyan
        "\x1b[0m",   // GAME : Normal default color
        "\x1b[32m",  // INFO : Green
        "\x1b[33m",  // WARN : Yellow
        "\x1b[31m",  // ERROR : Red
        "\x1b[35m",  // FATAL : Magenta
};

static const char *log_names[] = {
    "TRACE", "DEBUG", "GAME", "INFO", "WARN", "ERROR", "FATAL"
};

/**
 * @brief Logs a formatted message with metadata using a variadic argument list.
 *
 * This function facilitates logging operations where variadic arguments (`va_list`) are used
 * to dynamically format the message. It performs sanity checks, formats the message,
 * appends log metadata (e.g., log level, file, and line number), and writes the log entry
 * to the appropriate stream.
 *
 * @param level   The severity level of the log message (e.g., LOG_TRACE, LOG_INFO, LOG_ERROR).
 *                Must be a valid log level defined in the logging system.
 * @param file    The name of the source file where the log function was called.
 *                Typically provided as `__FILE__` in the caller.
 * @param line    The line number in the source file where the log function was called.
 *                Typically provided as `__LINE__` in the caller.
 * @param message The format string for the log message, similar to printf.
 *                Must be a valid null-terminated string.
 * @param args    A `va_list` containing the variadic arguments for the format string.
 *
 * @return
 * - `0`: Logging was skipped (e.g., level below the minimum threshold).
 * - `-1`: An error occurred (e.g., invalid log level, null message, or memory allocation failure).
 * - `>0`: Number of characters written to the log stream.
 *
 * Example usage:
 * @code
 * va_list args;
 * va_start(args, format);
 * logVA(LOG_INFO, __FILE__, __LINE__, "User %s logged in successfully", args);
 * va_end(args);
 * @endcode
 *
 * @warning
 * - Ensure that the `level` parameter is valid and within the defined log levels.
 * - The `message` parameter must not be NULL. Pass a valid format string.
 * - This function dynamically allocates memory for the formatted message. The allocated memory
 *   is freed internally before the function returns.
 * - The `args` parameter must be initialized using `va_start` before calling this function and
 *   cleaned up with `va_end` after use.
 *
 * @note
 * - The function uses the following helper functions:
 *   - `logSanityChecks`: Validates the log level and message.
 *   - `logFormatMessage`: Dynamically formats the message string using `va_list`.
 *   - `logGetStream`: Determines the appropriate log stream (e.g., stdout, stderr).
 *   - `logPrintHeader`: Writes log metadata (e.g., timestamp, log level).
 *   - `logPrintMessage`: Writes the formatted message to the log stream.
 *
 * @see logSanityChecks, logFormatMessage, logGetStream, logPrintHeader, logPrintMessage
 */

int logVA(int level, const char *file, int line, const char *message, va_list args) {

    // Init return value and sanity check inputs
    int ret = logSanityChecks(level, message);
    if (ret < 1) {
        // Something went wrong :(
        return ret;
    }

    // Format the message
    LogFormattedMessage formattedMessage = logFormatMessage(message, args);

    FILE * stream = logGetStream();
    logPrintHeader(stream, level);

    ret = logPrintMessage(stream, formattedMessage, level);

    // Cleanup
    free(formattedMessage.message);

    return ret;
}

/**
 * @brief Logs a formatted message with metadata to a specific stream.
 *
 * This function handles logging operations by performing sanity checks,
 * formatting the message, and appending log metadata (e.g., log level, file, and line number).
 * It writes the formatted log entry to the appropriate stream, which is determined
 * based on the log level.
 *
 * @param level   The severity level of the log message (e.g., LOG_TRACE, LOG_INFO, LOG_ERROR).
 *                Must be a valid log level defined in the logging system.
 * @param file    The name of the source file where the log function was called.
 *                Typically provided as `__FILE__` in the caller.
 * @param line    The line number in the source file where the log function was called.
 *                Typically provided as `__LINE__` in the caller.
 * @param message The format string for the log message, similar to printf.
 *                Must be a valid null-terminated string.
 * @param ...     Additional arguments for the format string.
 *
 * @return
 * - `0`: Logging was skipped (e.g., level below the minimum threshold).
 * - `-1`: An error occurred (e.g., invalid log level, null message, or memory allocation failure).
 * - `>0`: Number of characters written to the log stream.
 *
 * Example usage:
 * @code
 * logGeneric(LOG_INFO, __FILE__, __LINE__, "User %s logged in successfully", username);
 * @endcode
 *
 * @warning
 * - Ensure that the `level` parameter is valid and within the defined log levels.
 * - The `message` parameter must not be NULL. Pass a valid format string.
 * - This function allocates memory for the formatted message. The allocated memory
 *   is freed internally before the function returns.
 *
 * @note
 * - The function uses the following helper functions:
 *   - `logSanityChecks`: Validates the log level and message.
 *   - `logFormatMessage`: Formats the message string dynamically.
 *   - `logGetStream`: Determines the appropriate log stream (e.g., stdout, stderr).
 *   - `logPrintHeader`: Prints the log metadata (e.g., timestamp, log level).
 *   - `logPrintMessage`: Writes the formatted message to the stream.
 *
 * @see logSanityChecks, logFormatMessage, logGetStream, logPrintHeader, logPrintMessage
 */
int logGeneric(int level, const char *file, int line, const char *message, ...) {

    // Init return value and sanity check inputs
    int ret = logSanityChecks(level, message);
    if (ret < 1) {
        // Something went wrong :(
        return ret;
    }


    // Format the message
    va_list args;
    va_start(args, message);
    LogFormattedMessage formattedMessage = logFormatMessage(message, args);
    va_end(args);

    FILE * stream = logGetStream();
    logPrintHeader(stream, level);

    ret = logPrintMessage(stream, formattedMessage, level);

    // Cleanup
    free(formattedMessage.message);

    return ret;
}

/**
 * @brief Prints a formatted log message to the specified stream, handling message repetition.
 *
 * This function handles the output of log messages, including detecting repeated messages
 * and displaying a repeat counter instead of duplicating the same message. It also ensures
 * proper formatting of the message based on its content (e.g., whether it ends with a newline).
 *
 * @param stream            The output stream to write the log message to (e.g., stdout, stderr, or a file).
 * @param formattedMessage  A structure containing the formatted log message, its metadata, and repeat count.
 *                          - `message`: The formatted log message string.
 *                          - `repeat`: The number of times the message has been repeated.
 *                          - `endWithNewLine`: Flag indicating if the message ends with a newline character.
 * @param level             The log level of the message, used for printing headers if needed.
 *
 * @return The number of characters written to the stream, or a negative value if an error occurs.
 *
 * @warning
 * - Ensure `stream` is a valid `FILE*` pointer.
 * - The `formattedMessage` must be properly initialized with valid data before calling this function.
 *
 * @note
 * - If `LOG_NO_REPEAT` is enabled and the message has been repeated, the function clears the current
 *   line and rewrites the header with a repeat counter.
 * - The `logPrintHeader` function is called to reprint the log metadata when handling repeated messages.
 *
 * Example usage:
 * @code
 * LogFormattedMessage msg = {"This is a test message", 0, 1};
 * logPrintMessage(stdout, msg, LOG_INFO);
 * @endcode
 *
 * @see logPrintHeader
 */
int logPrintMessage(FILE *stream, LogFormattedMessage formattedMessage, int level) {
    int ret = 0;
    if (LOG_NO_REPEAT && formattedMessage.repeat > 0) {
        if (formattedMessage.repeat > 1) {
            // Not the first repeat
            // We clear current line to delete the header
            printf("\r\033[K");
            // We go up one line, go first char and clear it
            printf("\033[1A\r\033[K");
            // Reprint the header
            logPrintHeader(stream, level);
        }
        // The message is the same as the last one, we clear line and show a counter
        ret = fprintf(stream, "(last message repeated %i times)\n", formattedMessage.repeat);
    } else {
        if (formattedMessage.endWithNewLine == 1) {
            ret = fputs(formattedMessage.message, stream);
        } else {
            ret = fprintf(stream, "%s\n", formattedMessage.message);
        }
    }

    return ret;
}



/**
 * @brief Prints a formatted header to a given output stream.
 *
 * This function prints a log header that includes styling, a placeholder for
 * the current time, the log level, and resets the styling. The function is
 * designed to format log messages with consistent headers.
 *
 * @param stream The output stream to which the header will be printed (e.g., stdout or a file).
 * @param level  The log level (integer) that determines the style and level printed.
 *               The `log_style` array must be defined externally and accessible.
 *
 * @note The `log_style` array must contain strings for each log level, and the
 *       `level` parameter must be within its bounds to avoid undefined behavior.
 *
 * Example usage:
 * @code
 * logPrintHeader(stdout, 2);
 * @endcode
 */
int logPrintHeader(FILE *stream, int level) {
    // Print the style associated with the log level
    if (LOG_COLOR_ENABLED) {
        fprintf(stream, "%s", log_style[level]);
    }

    if (LOG_TIME_ENABLED) {
        /*
        // Get the current time
        time_t now = time(NULL);
        char time_buffer[20];
        struct tm *time_info = localtime(&now);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

        // Print a placeholder for the current time
        fprintf(stream, "%s ", time_buffer);
         */

        // Get elapsed time
        long seconds, milliseconds;
        logGetElapsedTime(&seconds, &milliseconds);

        // Print a placeholder for the current time
        fprintf(stream, "[%04ld.%03ld] ", seconds, milliseconds);
    }

    // Print the log level with a separator
    fprintf(stream, "%s> ", log_names[level]);

    // Clear styling to ensure subsequent text is not styled
    if (LOG_COLOR_ENABLED) {
        fprintf(stream, "%s", "\x1b[0m");
    }

    return 0; // Indicate success
}

/**
 * @brief Performs sanity checks on the log parameters before logging.
 *
 * This function validates the log level and the message string to ensure
 * that the logging operation is valid and meets the minimum logging level
 * requirements. It prevents invalid or unnecessary logging operations.
 *
 * @param level   The logging level of the message (e.g., LOG_TRACE, LOG_INFO).
 *                Must be within the range [LOG_TRACE, LOG_FATAL].
 * @param message The log message to validate. Must not be NULL.
 *
 * @return -1 if the level is invalid or the message is NULL,
 *          0 if the level is below the minimum logging threshold,
 *          1 if the parameters are valid and logging can proceed.
 *
 * Example usage:
 * @code
 * if (logSanityChecks(LOG_INFO, "This is a log message") > 0) {
 *     // Perform logging
 * }
 * @endcode
 */
int logSanityChecks(int level, const char *message) {
    // Check if the level is within the valid range
    if (level < LOG_TRACE || level > LOG_FATAL) {
        log_warn("Invalid level in log."); // Log a warning for invalid level
        return -1; // Indicate failure
    }

    // Suppress logging if the level is below the minimum threshold
    if (level < LOG_MIN_LEVEL) {
        return 0; // Indicate no logging needed
    }

    // Check if the message is NULL (unexpected scenario)
    if (!message) {
        log_warn("Called log with a NULL message."); // Log a warning for NULL message
        return -1; // Indicate failure
    }

    // If all checks pass, return success
    return 1;
}

/**
 * @brief Returns a FILE* stream initialized to stdout.
 *
 * This function returns a pointer to the standard output stream (stdout),
 * which can be used for logging or other output operations.
 *
 * @return A FILE* pointing to the stdout stream.
 */
FILE* logGetStream() {
  return stdout;
}

 /**
 * @brief Formats a message string with variadic arguments and returns both the string and its length.
 *
 * This function formats a message using a `va_list` of arguments and returns the result
 * as a dynamically allocated string along with its length. The caller is responsible for
 * freeing the returned string.
 *
 * @param message The format string, similar to a printf format string.
 *                Must be a valid null-terminated C string.
 * @param args    A `va_list` containing the variadic arguments for the format string.
 *
 * @return A `LogFormattedMessage` containing the dynamically allocated string and its length.
 *         If memory allocation fails, both fields in the structure will be set to 0 or NULL.
 *
 * @warning
 * - Ensure `message` is a valid format string. Passing NULL or an invalid format string
 *   may cause undefined behavior.
 * - The returned string must be freed by the caller to avoid memory leaks.
 * - Do not modify the returned string directly; treat it as read-only.
 * - Ensure `va_list` is correctly initialized before calling this function.
 *
 * @note This function uses `malloc` for memory allocation. Use it carefully in memory-constrained environments.
 */
LogFormattedMessage logFormatMessage(const char *message, va_list args) {
    LogFormattedMessage result = {NULL, 0}; // Initialize result with default values

    if (!message) {
        return result; // Return empty result if the format string is NULL
    }

    va_list args_copy;
    va_copy(args_copy, args);

    // Calculate the size of the formatted message (excluding the null terminator)
    size_t size = vsnprintf(NULL, 0, message, args_copy);
    va_end(args_copy);

    // Allocate memory for the formatted message
    char *formatted_message = (char *)malloc(size + 1); // +1 for the null terminator
    if (formatted_message == NULL) {
        return result; // Return empty result on allocation failure
    }

    // Format the message into the allocated memory
    vsnprintf(formatted_message, size + 1, message, args);

    // Detect line ending
    int ends_with_newline = 0;
    if (formatted_message[size - 1] == '\n' || formatted_message[size - 1] == '\r') {
        ends_with_newline = 1;
    }


    // Only log message if smaller than buffer
    if (LOG_NO_REPEAT && (size + 1) < LOG_REPEAT_BUFFER_SIZE) {
        // Compare previous and current messages
        if (strcmp(logLastMessage, formatted_message) == 0) {
            // Message is the same as the last one !
            logRepeatCount++;
        } else {
            // Track the new message.
            strncpy(logLastMessage, formatted_message, size + 1);
            // Force a null terminator - is this needed ?
            // logLastMessage[LOG_REPEAT_BUFFER_SIZE - 1] = '\0';
        }

    } else {
        // Message bigger than buffer, set counter to 0
        logRepeatCount = 0;
    }



    // Populate the result structure
    result.message = formatted_message;
    result.size = size;
    result.endWithNewLine = ends_with_newline;
    result.repeat = logRepeatCount;

    return result;
}

/**
 * @brief Initializes the timer at the start of the program.
 */
void logInitTimer() {
    clock_gettime(CLOCK_MONOTONIC, &logStartTime);
}

/**
 * @brief Gets the elapsed time in seconds and milliseconds since the program started.
 *
 * @param seconds Pointer to store the seconds part of the elapsed time.
 * @param milliseconds Pointer to store the milliseconds part of the elapsed time.
 */
void logGetElapsedTime(long *seconds, long *milliseconds) {
    struct timespec now;
    if (logStartTime.tv_sec == 0) {
        logInitTimer();
    }
    clock_gettime(CLOCK_MONOTONIC, &now);

    *seconds = now.tv_sec - logStartTime.tv_sec;
    *milliseconds = (now.tv_nsec - logStartTime.tv_nsec) / 1e6;

    if (*milliseconds < 0) {
        *milliseconds += 1000;
        (*seconds)--;
    }
}
