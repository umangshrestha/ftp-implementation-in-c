#include <stdio.h>  //fprinf, stderr, std, out
#include <time.h>   // time_t, time
#include <stdlib.h> // exit, EXIT_FAILURE
#include <stdarg.h> // va_list, va_start, va_end
#include "log.h"
// COLOR
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define NO_COLOR "\033[0m"
#define SIZE 19

void debugf(const char *, ...);
void infof(const char *, ...);
void warnf(const char *, ...);

void null(const char msg[], ...){};

Log *new_logger(LoggingLevel level)
{
    Log *log = (Log *)malloc(sizeof(Log));
    log->warnf = &warnf;
    log->infof = (level < Warn) ? &infof : &null;
    log->debugf = (level == Debug) ? &debugf : &null;
    return log;
}

void logging_format(const char *color, const char *level)
{

    time_t now;
    struct tm *time_info;
    time(&now);
    time_info = localtime(&now);
    // adding color
    fprintf(stderr,  "%s", color);
    // printing strftime
    fprintf(stderr, "[%d/%d/%d %d:%d:%04d]", time_info->tm_year + 1900,
            time_info->tm_mon + 1,
            time_info->tm_mday,
            time_info->tm_hour,
            time_info->tm_min,
            time_info->tm_sec);
    // printf tag
    fprintf(stderr, "[%s]", level);
}
void debugf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logging_format(YELLOW, "DEBUG");
    // print data
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "%s", NO_COLOR);
    va_end(args);
}

void infof(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logging_format(GREEN, "INFO");
    // print data
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "%s", NO_COLOR);
    va_end(args);
}

void warnf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logging_format(RED, "WARN");
    // print data
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "%s", NO_COLOR);
    va_end(args);
}