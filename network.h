/*
 * network.h
 *
 *  Created on: 2017年1月12日
 *      Author: root
 */

#ifndef NETWORK_H_
#define NETWORK_H_
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
#define Frame_Header_First 0x7E
#define Frame_Header_Second 0x7F
#define Ack_ID 0x11
#define Update_Request_ID 0x12
#define Send_Interval_Request_ID 0x13
#define Set_Interval_Request_ID 0x14
void connect_tcp();
void init_ppp();
void check_ppp();
void recv_analysis();
void anlysis(char *buffer,int *len);

#endif /* NETWORK_H_ */
