/*
 * network.cpp
 *
 *  Created on: 2017年1月12日
 *      Author: root
 */
#include "network.h"
#include "types.h"
#include "upload.h"
#include "upgrade.h"
#include "log.h"
#include <iostream>
#include <fstream>
using namespace std;
int sockfd1;
int ppp_flag=0;
int socket_flag=0;
int recv_error=0;
char recv_buffer[1024]={0};
char recv_anlysis_buffer[1024]={0};
int recv_anlysis_buffer_len=0;
int reconnect_tcp=0;
extern uint16_t GPS_interval;
extern uint16_t GPS_interval_count;
extern bool detect_flag;
extern log_st *log_sleep;
extern bool detect_or_save_flag;
extern bool aplay_flag;
void connect_tcp()
{
	DBG("function:connect_tcp\r\n");
	LogDebug(log_sleep, "function:connect_tcp\r\n");
	reconnect_tcp++;
	DBG("reconnect tcp %d\r\n",reconnect_tcp);
	if(reconnect_tcp==30)
	{
		LogDebug(log_sleep, "reconnect tcp %d\r\n",reconnect_tcp);
		system("reboot");
	}
	struct sockaddr_in servaddr;
	sockfd1 = socket(PF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server_port1);
	servaddr.sin_addr.s_addr = inet_addr(server_IP);
	unsigned long ul = 1;
	ioctl(sockfd1, FIONBIO, &ul); //设置为非阻塞模式
	/*if(connect(sockfd1, (struct sockaddr *)&servaddr, sizeof(servaddr))==0)
	{
		recv_error=0;
		send_register(Device_ID);
		socket_flag=1;
	}*/

	connect(sockfd1, (struct sockaddr *)&servaddr, sizeof(servaddr));

	recv_error=0;
	sleep(2);
	send_register(Device_ID);
	socket_flag=1;

	ul = 0;
	ioctl(sockfd1, FIONBIO, &ul); //设置为阻塞模式


	struct timeval timeout={1,0};//1s
	setsockopt(sockfd1,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
	setsockopt(sockfd1,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));

}
void init_ppp()
{
	DBG("function:init_ppp\r\n");
	system("pppd call wcdma &");
	//system("pppd call cdma2000 &");
	sleep(5);
	int a=system("ifconfig | grep ppp0");
	if(a==0)
	{
		ppp_flag=1;
		//cout<<"successful"<<endl;
		DBG("ppp0 connect\r\n");
	}
}
void check_ppp()
{
	DBG("function:check_ppp\r\n");
	int a=system("ifconfig | grep ppp0");
	#ifdef _PC
		a=0;
	#endif
	if(a==0)
	{
		//cout<<"connect"<<endl;
		DBG("ppp0 connect\r\n");
		a=0;
	}
	else
	{
		//cout<<"relink"<<endl;
		DBG("relink ppp\r\n");
		LogDebug(log_sleep, "relink ppp\r\n");
		system("kill -9 `pidof pppd call wcdma`");
		sleep(1);
		system("pppd call wcdma &");
		//system("pppd call cdma2000 &");
		sleep(15);
		connect_tcp();
		sleep(1);
		send_register(Device_ID);
	}
	if(socket_flag==0)
	{
		shutdown(sockfd1,2);
		//cout<<"socket error!!!"<<endl;
		DBG("socket error\r\n");
		connect_tcp();
	}
	sleep(1);
}
void recv_analysis()
{
	DBG("detect_flag=%d\r\n",detect_flag);
	//cout<<"recv"<<endl;
	DBG("function:recv_analysis\r\n");
	DBG("recv_error = %d\r\n",recv_error);
	if(socket_flag==1)
	{
		int recv_len=recv(sockfd1,recv_buffer,1024,0);
		//cout<<recv_len<<endl;
		DBG("recv_len=%d\r\n",recv_len);
		if(recv_len==-1)
		{
			DBG("receive error:errno=%d\r\n",errno);
			DBG("receive error: %s\r\n",strerror(errno));
			if(errno!=11)
			{
				LogDebug(log_sleep, "receive error:errno=%d\r\n",errno);
				LogDebug(log_sleep, "receive error: %s\r\n",strerror(errno));
			}
		}
		if(recv_len<=0)
		{
			recv_error++;
			if(recv_error>=receive_error_time)
			{
				socket_flag=0;
				shutdown(sockfd1,2);
				close(sockfd1);
			}
		}
		else
		{
			recv_error=0;
			reconnect_tcp=0;
			for(int i=0;i<recv_len;i++)
			{
				recv_anlysis_buffer[recv_anlysis_buffer_len+i]=recv_buffer[i];
			}
			recv_anlysis_buffer_len=recv_anlysis_buffer_len+recv_len;
			anlysis(recv_anlysis_buffer,&recv_anlysis_buffer_len);
		}
	}
	else
	{
		sleep(1);
	}
}

void anlysis(char *buffer,int *len)
{
	DBG("network_anlysis:\r\n");
	for(int i=0;i<(*len);i++)
	{
		DBG("%02x\r\n",buffer[i]);
	}
	int i=0;
	int flag=0;
	uint32_t frame_len=0;
	int complete_len=0;
	for(i=0;i<(*len);i++)
	{
		if(buffer[i]==Frame_Header_First && flag==0)
		{
			flag=1;
		}
		else if(buffer[i]==Frame_Header_Second && flag==1)
		{
			flag=2;
		}
		else if(flag==2 && *len>=6)
		{
			frame_len=(buffer[i]<<24)+(buffer[i+1]<<16)+(buffer[i+2]<<8)+(buffer[i+3]);
			flag=3;
			i=i+3;
		}
		else if(flag==3 && (*len-complete_len)>=frame_len)
		{
			if(buffer[i]==Ack_ID)
			{
				//cout<<"ack"<<endl;
				DBG("ack\r\n");
			}
			else if(buffer[i]==Update_Request_ID)
			{
				//cout<<"update request"<<endl;
				DBG("update request\r\n");
				UpgradeApp();
			}
			else if(buffer[i]==Send_Interval_Request_ID)
			{
				//cout<<"send interval"<<endl;
				DBG("send interval request\r\n");
				send_gps_interval(Device_ID,GPS_interval);
			}
			else if(buffer[i]==Set_Interval_Request_ID)
			{
				//cout<<"set interval"<<endl;
				DBG("set interval request\r\n");
				GPS_interval=(uint8_t)buffer[i+1]*256+(uint8_t)buffer[i+2];
				GPS_interval_count=0;
			}
			else if(buffer[i]==0x15)
			{
				i++;
				if(buffer[i]==0x00)
				{
					//cout<<"turn off detect"<<endl;
					DBG("turn off detect\r\n");
					detect_flag=false;
					system("aplay alarm.wav &");
					//pthread_cancel(ntid1);
				}
				else
				{
					//cout<<"turn on detect"<<endl;
					DBG("turn on detect\r\n");
					detect_flag=true;
					//pthread_create(&ntid1,NULL,thr_fn_detect,NULL);
				}
			}
			else if(buffer[i]==0x17)
			{
				DBG("detect or save change\r\n");
				i++;
				if(buffer[i]==0)
				{
					DBG("change to detect\r\n");
					detect_or_save_flag=false;
				}
				else if(buffer[i]==1)
				{
					DBG("change to save video\r\n");
					detect_or_save_flag=true;
				}
				ofstream out("detect_save_flag.txt");
				if(!out.is_open())
				{
					DBG("Open detect_save_flag.txt write failed\r\n");
					LogDebug(log_sleep, "Open detect_save_flag.txt write failed\r\n");
				}
				else
				{
					DBG("write detect_save_flag : %d\r\n",detect_or_save_flag);
					out<<detect_or_save_flag;
				}
				out.close();
				out.clear();
			}
			else if(buffer[i]==0x19)
			{
				DBG("enter dangrous area\r\n");
				system("aplay alarm.wav &");
			}
			else if(buffer[i]==0x1a)
			{
				DBG("alarm flag change\r\n");
				i++;
				if(buffer[i]==0)
				{
					DBG("shutdown aplay alarm.wav\r\n");
					aplay_flag=false;
				}
				else if(buffer[i]==1)
				{
					DBG("open aplay alarm.wav\r\n");
					aplay_flag=true;
				}
				ofstream out("aplay_flag.txt");
				if(!out.is_open())
				{
					DBG("Open aplay_flag.txt write failed\r\n");
					LogDebug(log_sleep, "Open aplay_flag.txt write failed\r\n");
				}
				else
				{
					DBG("write aplay_flag : %d\r\n",aplay_flag);
					out<<aplay_flag;
				}
				out.close();
				out.clear();
			}
			complete_len=complete_len+frame_len;
			flag=0;
		}
		else
		{
			flag=0;
		}
	}
	for(i=complete_len;i<(*len);i++)
	{
		buffer[i-complete_len]=buffer[i];
	}
	*len=*len-complete_len;
}
