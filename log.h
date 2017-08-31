#ifndef _LOG_H_
#include "types.h"
//#include "config.h"
#define _LOG_H_

#define LOG_BUF_SIZE 1024
typedef struct _log_st  
{  
    s8 path[128];  
    int fd;  
    int size;  
}log_st;   
log_st *LogInit(s8 *path, int size);
//log_st *log_g;
void LogDebug(log_st *log, const s8 *msg, ...);
void LogCheckSize(log_st *log);
#endif
