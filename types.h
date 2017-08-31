#ifndef _TYPES_H_
#define _TYPES_H_



typedef long s32;
typedef short s16;
typedef unsigned short u16,uint16_t;
typedef unsigned int u32,uint32_t;
typedef char s8;
typedef unsigned char u8,uint8_t;
typedef long long s64;
typedef unsigned long long u64,uint64_t;




#define MS 1000    

#define TRUE             1
#define FALSE           0

#define PHOTO           1
#define VIDEO            0

#define MSALARM          0x10

#define BUFFER_FULL     1
#define BUFFER_DATA     2
#define BUFFER_EMPTY    3

#define KB              1024
#define MB              1024*1024


//#define _PC//电脑虚拟机使用时定义，板子使用时注释掉
#define _GPS
//#define _NO_GPS
#define _RTK

#define save_path "/mnt/sleep/"

//#define Device_ID 10002
extern uint32_t Device_ID;
//version:Software_Version_first.Software_Version_second
#define Software_Version_first 1
#define Software_Version_second 0
#define receive_error_time 60 //60秒无下行数据进行重连

/*#define server_IP "119.78.249.27"
#define server_port1 1900
#define server_port2 1910*/
#define server_IP "117.144.208.162"
#define server_port1 5895
#define server_port2 5896
/*#define server_IP "172.20.19.114"
#define server_port1 8010
#define server_port2 8020*/
/*#define server_IP "172.20.16.48"
#define server_port1 8010
#define server_port2 8020*/
#ifdef _PC
	#define gps_port "/dev/ttyUSB0"
#else
	#define gps_port "/dev/ttymxc2"
#endif
#ifdef _PC
	#define rtk_port "/dev/ttyUSB0"
#else
	#define rtk_port "/dev/ttymxc2"
#endif

void init_(void);
void mainloop(void);

#define __DEBUG

#ifdef __DEBUG
	#define DBG(fmt, args...) fprintf(stderr, "Sleep Debug: " fmt, ## args)
#else
	#define DBG(fmt, args...)
#endif

//#include "log.h"
//extern log_st *log_sleep;

#endif

