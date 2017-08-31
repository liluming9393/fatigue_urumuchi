#include <stdio.h>	
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/select.h>	
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "log.h"
#include "types.h"

log_st *LogInit(s8 *path, int size)  
{  
	DBG("log file size %d\r\n",size);
	s8 new_path[128] = {0};  
	if (path==NULL )
	     return NULL;  	
	log_st *log = (log_st *)malloc(sizeof(log_st));	
	memset(log, 0, sizeof(log_st));	 
	snprintf(new_path, 128, "%ssleep.log1", path);  
    
	if(-1 == (log->fd = open(new_path, O_RDWR|O_APPEND|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR|S_IROTH)))  
	{  
	    free(log);  
	    log = NULL;  
	    return NULL;  
	}        
	strncpy(log->path, path, 128);  
	log->size = (size > 0 ? size:0);       
	return log;  
}  

 void LogCheckSize(log_st *log)  
{  
   struct stat stat_buf;  
   s8 new_path[128] = {0};  
   s8 bak_path[128] = {0};  
    if(NULL == log || '\0' == log->path[0])
        return;  
    memset(&stat_buf, 0, sizeof(struct stat));  
    fstat(log->fd, &stat_buf);  
    if(stat_buf.st_size > log->size)  
    {      
    	DBG("backup log file\r\n");
        close(log->fd);    
        snprintf(bak_path, 128, "%ssleep.log2", log->path);  
        snprintf(new_path, 128, "%ssleep.log1", log->path);  
        remove(bak_path); //delete the file *.log2 first  
        rename(new_path, bak_path); //change the name of the file *.log1to *.log2         
        //create a new file  
        log->fd = open(new_path, O_RDWR|O_APPEND|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR|S_IROTH);  
     }  
}  


void LogDebug(log_st *log, const s8 *msg, ...)  
{  
	int mount_check=system("ls /dev/ | grep mmcblk2p1");
	#ifdef _PC
		mount_check=0;
	#endif
	DBG("log mount_check %d \r\n",mount_check);
	if(mount_check!=0)
	{
		//system("umount -l /mnt/sleep &");
		system("mount -t ext4 /dev/mmcblk2p1 /mnt/sleep");
		return;
	}
	va_list ap;  
	time_t now;  
	s8 *pos;  
	s8 _n = '\n';  
	s8 message[LOG_BUF_SIZE] = {0};  
	int nMessageLen = 0;  
	int sz;  
	if(NULL == log)
          return;  
	now = time(NULL);  
	pos = ctime(&now);  
	sz = strlen(pos);
	
	pos[sz-1]=']';  
	snprintf(message, LOG_BUF_SIZE , "[%s ", pos);  
	for (pos = message; *pos; pos++);  
	     sz = pos - message;  
	va_start(ap, msg);  
	nMessageLen = vsnprintf(pos, LOG_BUF_SIZE - sz, msg, ap);  
	va_end(ap);  
	if (nMessageLen <= 0)
            return;  	  
	write(log->fd, message, strlen(message)); 
	write(log->fd, &_n, 1);  
	fsync(log->fd); 
    
      LogCheckSize(log);  
       
}




