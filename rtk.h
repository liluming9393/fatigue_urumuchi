/*
 * rtk.h
 *
 *  Created on: 2017年4月6日
 *      Author: root
 */

#ifndef RTK_H_
#define RTK_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <iostream>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include "types.h"

using namespace std;

struct RTK_info
{
	double Longitude;//经度
	double Latitude;//纬度
	float v;//地面速率
	float heading;//地面航向
	uint8_t status;//GPS状态״̬
	uint64_t second;//unix时间
	uint32_t time;
	uint32_t date;
};
void rtk_init();
void setTermios_rtk(struct termios * pNewtio, int uBaudRate);
void connect_rtk_result_server();
void rtk_recv_analysis();
void find_str_rtk(char* buffer, char* str, uint32_t buffer_lengh);
uint8_t GPS_analysis_rtk(char* gps_buffer);


#endif /* RTK_H_ */
