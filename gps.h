/*
 * gps.h
 *
 *  Created on: 2017年2月14日
 *      Author: root
 */

#ifndef GPS_H_
#define GPS_H_

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/signal.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>
#include"types.h"
#include<ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define flag 1
#define noflag 0

struct GPS_info
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
void connect_rtk_input_server();
void setTermios(struct termios * pNewtio, int uBaudRate);
int gps_recv();
uint8_t GPS_analysis(char* gps_buffer);
void find_str(char* buffer, char* str, uint32_t buffer_length);

#endif /* GPS_H_ */
