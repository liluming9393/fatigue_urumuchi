#ifndef _WDOG_H_
#define _WDOG_H_

   

#define WDT_DOG_TIMEOUT      30    
#define WDT_DOG_SLEEP        10  
#define WDT_DOG_TEST         0//0 - Service wdt with ioctl(), 1 - with write()

int RunWdog(int timeout,int sleep_sec, int test);

#endif
