#ifndef __LOG_H
#define __LOG_H

enum LoggingLevel
{
    Debug,
    Info,
    Warn
};
typedef int LoggingLevel;
typedef struct LOG Log;

struct LOG
{
    void (*debugf)(const char *, ...);
    void (*infof)(const char *, ...);
    void (*warnf)(const char *, ...);
};

Log * new_logger( LoggingLevel);

#endif