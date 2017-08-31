#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "types.h"
#include "log.h"
extern log_st *log_sleep;
//void help_info(void);

int RunWdog(int timeout,int sleep_sec, int test)
{
    int fd;
    //if (argc < 2) {
   //     help_info();
    //    return 1;
   // }
  //  timeout = atoi(argv[1]);
   // sleep_sec = atoi(argv[2]);
    if (sleep_sec <= 0) {
        sleep_sec = 1;
        printf("correct 0 or negative sleep time to %d seconds\n",
               sleep_sec);
    }
 //   test = atoi(argv[3]);
    printf("Starting wdt_driver (timeout: %d, sleep: %d, test: %s)\n",
           timeout, sleep_sec, (test == 0) ? "ioctl" : "write");
    fd = open("/dev/watchdog", O_WRONLY);
    if (fd == -1) {
        perror("watchdog");
        LogDebug(log_sleep, "open watchdog error\r\n");
        system("reboot");
        exit(1);
    }
    DBG("Trying to set timeout value=%d seconds\n", timeout);
    ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
    DBG("The actual timeout was set to %d seconds\n", timeout);
    ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
    DBG("Now reading back -- The timeout is %d seconds\n", timeout);
    while (1) {
    	DBG("feed watchdog\r\n");
        if (test == 0) {
            ioctl(fd, WDIOC_KEEPALIVE, 0); //ʹ��ioctl()����ι��
        } else {
            write(fd, "\0", 1); //ʹ��write()����ι����
        }
        sleep(sleep_sec);
    }
    return 0;
}
