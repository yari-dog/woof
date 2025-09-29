#ifndef UTIL_H
#define UTIL_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int create_shm_file(int size);

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define PRINTFUNCTION(format, ...) fprintf(stderr, format, __VA_ARGS__)
#define LOG_ARGS(LOG_TAG) timenow(), LOG_TAG
#define LOG_FMT(LOG_COLOR) "(%s)" LOG_COLOR "  %s "
#define NEWLINE "\n"

static inline char *timenow() {
    static char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 64, "%H:%M:%S", timeinfo);

    return buffer;
}

#define INFO(message, args...)                                                                                         \
    PRINTFUNCTION(LOG_FMT(ANSI_COLOR_YELLOW) message ANSI_COLOR_RESET NEWLINE, LOG_ARGS("INFO:"), ##args)

#define EXIT(message, args...)                                                                                         \
    {                                                                                                                  \
        PRINTFUNCTION(LOG_FMT(ANSI_COLOR_RED) message ANSI_COLOR_RESET NEWLINE, LOG_ARGS("ERROR:"), ##args);           \
        exit(errno);                                                                                                   \
    }

#define OUT_MESSAGE(message, args...)                                                                                  \
    PRINTFUNCTION(LOG_FMT(ANSI_COLOR_CYAN) message ANSI_COLOR_RESET NEWLINE, LOG_ARGS("->"), ##args)

#define IN_MESSAGE(message, args...)                                                                                   \
    PRINTFUNCTION(LOG_FMT(ANSI_COLOR_CYAN) message ANSI_COLOR_RESET NEWLINE, LOG_ARGS("<-"), ##args)

#endif
