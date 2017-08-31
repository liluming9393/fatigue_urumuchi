#include "no4.h"
#include "gps.h"
#include "types.h"
#include "upload.h"
#include "process.h"
#include "network.h"
#include "rtk.h"
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <fstream>
#include "log.h"
#include "wdog.h"
#include "savecamera.h"
using namespace std;

pthread_t ntid0;//wdog
pthread_t ntid1;//detect
pthread_t ntid2;//net
pthread_t ntid3;//gps
pthread_t ntid4;//recv
pthread_t ntid5;//rtk

bool detect_or_save_flag=true;//false is detect, true is save
bool aplay_flag=false;//false : not play alarm.wav  true : play alarm.wav

extern int sleep_value;
extern float left_eye_value;
extern float right_eye_value;
extern int window_width;
extern int detect_time;
extern struct GPS_info gps_info;
//char gps_buffer[4096]="$GNRMC,065420.00,A,3123.25411,N,12113.85559,E,1.539,312.24,220416,,,D*7F\r\n";
int camera_error_flag=0;
pthread_mutex_t gps_lock;
void * thr_fn_wdog(void *arg);
void * thr_fn_detect(void *arg);//疲劳检测函数
void * thr_fn_net(void *arg);
void * thr_fn_gps(void *arg);
void * thr_fn_recv(void *arg);
void * thr_fn_rtk(void *arg);
uint32_t Device_ID=0;
log_st *log_sleep;
int main(int argc, char **argv)
{
	int err;
	#ifndef _PC
		DBG("create watchdog thread\r\n");
		err=pthread_create(&ntid0,NULL,thr_fn_wdog,NULL);
		if(err!=0)
		{
			DBG("can't create thread watchdog\r\n");
			LogDebug(log_sleep, "can't create thread watchdog\r\n");
			system("reboot");
			//cout<<"can't create thread 1"<<endl;
			return 0;
		}
	#endif
	DBG("open log file\r\n");
	log_sleep=LogInit(save_path, 1024*1024*500);
	//LogDebug(log_sleep, "log start\r\n");
	ifstream in("ID.txt");
	if(!in.is_open())
	{
		DBG("Open ID.txt failed\r\n");
		LogDebug(log_sleep, "Open ID.txt failed\r\n");
		//return -1;
	}
	else
	{
		in>>Device_ID;
	}
	in.close();
	in.clear();
	if(Device_ID==0)
	{
		DBG("ID error\r\n");
		LogDebug(log_sleep, "ID error\r\n");
		//return -1;
	}
	//ifstream in("detect_save_flag.txt");
	in.open("detect_save_flag.txt");
	if(!in.is_open())
	{
		DBG("Open detect_save_flag.txt read failed\r\n");
		LogDebug(log_sleep, "Open detect_save_flag.txt read failed\r\n");
	}
	else
	{
		int flag_read;
		in>>flag_read;
		if(flag_read==0)
		{
			detect_or_save_flag=false;
		}
		else
		{
			detect_or_save_flag=true;
		}
		DBG("read detect_save_flag : %d\r\n",flag_read);
	}
	in.close();
	in.clear();
	in.open("aplay_flag.txt");
	if(!in.is_open())
	{
		DBG("Open aplay_flag.txt read failed\r\n");
		LogDebug(log_sleep, "Open aplay_flag.txt read failed\r\n");
	}
	else
	{
		int flag_read;
		in>>flag_read;
		if(flag_read==0)
		{
			aplay_flag=false;
		}
		else
		{
			aplay_flag=true;
		}
		DBG("read aplay_flag : %d\r\n",flag_read);
	}
	in.close();
	in.clear();
	DBG("Software Version:%d.%d\r\n",Software_Version_first,Software_Version_second);
	DBG("Device ID:%d\r\n",Device_ID);
	DBG("Main Start\r\n");
	if (argc < 6) {
		//cout << "Please enter parameter:" << endl;
		DBG("Please enter parameter:\r\n");
		LogDebug(log_sleep, "Please enter parameter:\r\n");
		return 0;
	}
	else{
		DBG("Receive parameter correct\r\n");
		sleep_value=atoi(argv[1]);
		window_width=atoi(argv[2]);
		left_eye_value=atof(argv[3]);
		right_eye_value=atof(argv[4]);
		detect_time=atoi(argv[5]);
	}
	signal(SIGPIPE, SIG_IGN);
	pthread_mutex_init(&gps_lock,NULL);
	DBG("open save camera file\r\n");
	saveCamera();
	DBG("create detect thread\r\n");
	err=pthread_create(&ntid1,NULL,thr_fn_detect,NULL);
	if(err!=0)
	{
		DBG("can't create thread detect\r\n");
		LogDebug(log_sleep, "can't create thread detect\r\n");
		//cout<<"can't create thread 1"<<endl;
		return 0;
	}
	#ifndef _PC
		DBG("check ppp0\n");
		int a=system("ifconfig | grep ppp0");
		if(a!=0)
		{
			DBG("init_ppp\n");
			init_ppp();
		}
		sleep(15);
	#endif
	connect_tcp();
	sleep(1);
	DBG("create network thread\r\n");
	err=pthread_create(&ntid2,NULL,thr_fn_net,NULL);
	if(err!=0)
	{
		//cout<<"can't create thread 2"<<endl;
		DBG("can't create thread network\r\n");
		LogDebug(log_sleep, "can't create thread network\r\n");
		return 0;
	}
	#ifdef _RTK
		DBG("create rtk thread\r\n");
		err=pthread_create(&ntid5,NULL,thr_fn_rtk,NULL);
		if(err!=0)
		{
			//cout<<"can't create thread 4"<<endl;
			DBG("can't create thread rtk\r\n");
			LogDebug(log_sleep, "can't create thread rtk\r\n");
			return 0;
		}
	#endif
	DBG("create gps thread\r\n");
	err=pthread_create(&ntid3,NULL,thr_fn_gps,NULL);
	if(err!=0)
	{
		//cout<<"can't create thread 3"<<endl;
		DBG("can't create thread gps\r\n");
		LogDebug(log_sleep, "can't create thread gps\r\n");
		return 0;
	}
	DBG("create receive thread\r\n");
	err=pthread_create(&ntid4,NULL,thr_fn_recv,NULL);
	if(err!=0)
	{
		//cout<<"can't create thread 4"<<endl;
		DBG("can't create thread receive\r\n");
		LogDebug(log_sleep, "can't create thread receive\r\n");
		return 0;
	}


	while(1);
	return 1;
}
void * thr_fn_wdog(void *arg)
{
	RunWdog(WDT_DOG_TIMEOUT,WDT_DOG_SLEEP, WDT_DOG_TEST);
	return ((void *)0);
}
void * thr_fn_detect(void *arg)
{
	while(1)
	{
		detect();
	}
	return ((void *)0);
}
void * thr_fn_net(void *arg)
{
	while(1)
	{
		check_ppp();
	}
	return ((void *)0);
}
void * thr_fn_gps(void *arg)
{
	/*#ifdef _RTK
		//rtk_init();
		//sleep(2);
		DBG("start rtk single\r\n");
		system("./rtkrcv -s -o rtkrcv_single.conf &");
		//system("./rtkrcv -s -o rtkrcv_rtk.conf &");
		connect_rtk_result_server();
		while(1)
		{
			rtk_recv_anaysis();
		}
		return ((void *)0);
	#endif*/
	#ifdef _GPS
		gps_recv();
	#endif
	#ifdef _NO_GPS
		while(1)
		{
			struct timeval tv;
			struct timezone tz;
			gettimeofday(&tv,&tz);
			DBG("gps_time: %d\n",tv.tv_sec);
			#ifdef _PC
				gps_info.second = tv.tv_sec-8*3600;
			#else
				gps_info.second = tv.tv_sec-8*3600;
			#endif
			gps_info.Latitude=43.84117;
			gps_info.Longitude=87.46684;
			gps_info.v=20;
			gps_info.heading=0;
			send_gps(Device_ID,gps_info.second,gps_info.Longitude,gps_info.Latitude,gps_info.v,gps_info.heading,0);
			//send_gps(10003,tv.tv_sec,87.66413,43.96247,20,20);
			sleep(1);
		}
	#endif
	return ((void *)0);
}
void * thr_fn_recv(void *arg)
{
	while(1)
	{
		recv_analysis();
		//sleep(1);
	}
	return ((void *)0);
}
void * thr_fn_rtk(void *arg)
{
	//rtk_init();
	//sleep(2);
	DBG("start rtk single\r\n");
	system("./rtkrcv -s -o rtkrcv_single.conf &");
	sleep(2);
	//system("./rtkrcv -s -o rtkrcv_rtk.conf &");
	connect_rtk_result_server();
	while(1)
	{
		rtk_recv_analysis();
	}
	return ((void *)0);
}
