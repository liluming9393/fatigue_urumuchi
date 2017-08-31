#include "upload.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.h"
extern log_st *log_sleep;
extern bool detect_or_save_flag;
extern bool aplay_flag;
//#include <Winsock2.h>
//extern SOCKET sockclient;
//注册信令
void send_register(uint32_t device_id)
{
	DBG("function:send_register\r\n");
	uint8_t *sendbuffer=new uint8_t[12];
	struct upload_register data;
	DBG("send_register copy frame_head\r\n");
	data.frame_head = 0x7F7E;
	DBG("send_register copy len\r\n");
	data.len = 0x0C000000;
	DBG("send_register copy id\r\n");
	data.id = 0x91;
	DBG("send_register copy devide_id\r\n");
	data.device_id = ((device_id & 0xFF000000) >> 24) | ((device_id & 0x00FF0000) >> 8) | ((device_id & 0x0000FF00) << 8) | ((device_id & 0x000000FF) << 24);
	DBG("send_register copy reserve\r\n");
	data.reserve = 0;
	DBG("send_register memcpy\r\n");
	memcpy(sendbuffer, &data, 12);
	DBG("send_register before send\r\n");
	int send_len=send(sockfd1, (char *)sendbuffer, 12, 0);
	DBG("send_register send_len=%d\r\n",send_len);
	if(send_len==-1)
	{
		DBG("send_register send error:errno=%d\r\n",errno);
		DBG("send_register send error: %s\r\n",strerror(errno));
		LogDebug(log_sleep, "send_register send error:errno=%d\r\n",errno);
		LogDebug(log_sleep, "send_register send error: %s\r\n",strerror(errno));
	}
	DBG("send_register data:\r\n");
	for(int i=0;i<12;i++)
	{
		DBG("0x%02x\r\n",sendbuffer[i]);
	}
	free(sendbuffer);
}
//监测报警信息上报信令
void send_alarm(uint32_t device_id, uint64_t time, double longitude, double latitude, float v, float heading, uint8_t status)
{
	DBG("function:send_alarm\r\n");
	uint8_t *sendbuffer=new uint8_t[38];
	struct upload_alarm data;
	data.frame_head = 0x7F7E;
	data.len = 0x26000000;
	data.id = 0x92;
	data.device_id = ((device_id & 0xFF000000) >> 24) | ((device_id & 0x00FF0000) >> 8) | ((device_id & 0x0000FF00) << 8) | ((device_id & 0x000000FF) << 24);
	DBG("send alarm time %lld\r\n",time);
	data.time[5] = (time & 0x0000000000FF);
	data.time[4] = (time & 0x00000000FF00) >> 8;
	data.time[3] = (time & 0x000000FF0000) >> 16;
	data.time[2] = (time & 0x0000FF000000) >> 24;
	data.time[1] = (time & 0x00FF00000000) >> 32;
	data.time[0] = (time & 0xFF0000000000) >> 40;
	data.longitude = longitude;
	data.latitude = latitude;
	data.v = (uint16_t)(v * 100);
	data.v = ((data.v & 0xFF00) >> 8) | ((data.v & 0x00FF) << 8);
	data.heading = (uint16_t)(heading * 100);
	data.heading = ((data.heading & 0xFF00) >> 8) | ((data.heading & 0x00FF) << 8);
	data.status = status;
	memcpy(sendbuffer, &data, 38);
	int send_len=send(sockfd1, (char *)sendbuffer, 38, 0);
	DBG("send_alarm send_len=%d\r\n",send_len);
	if(send_len==-1)
	{
		DBG("send_alarm send error:errno=%d\r\n",errno);
		DBG("send_alarm send error: %s\r\n",strerror(errno));
		LogDebug(log_sleep, "send_alarm send error:errno=%d\r\n",errno);
		LogDebug(log_sleep, "send_alarm send error: %s\r\n",strerror(errno));
	}
	DBG("send_alarm data:\r\n");
	for(int i=0;i<38;i++)
	{
		DBG("0x%02x\r\n",sendbuffer[i]);
	}
	free(sendbuffer);
}
//图片上传信令
void send_picture(uint32_t device_id, uint64_t time, uint8_t picture_id, uint8_t* picture, uint32_t picture_len)
{
	DBG("function:send_picture\r\n");
	uint32_t frame_len = 18 + picture_len;
	uint8_t *sendbuffer=new uint8_t[frame_len];
	struct upload_picture data;
	data.frame_head = 0x7F7E;
	data.len = ((frame_len & 0xFF000000) >> 24) | ((frame_len & 0x00FF0000) >> 8) | ((frame_len & 0x0000FF00) << 8) | ((frame_len & 0x000000FF) << 24);
	data.id = 0x93;
	data.device_id = ((device_id & 0xFF000000) >> 24) | ((device_id & 0x00FF0000) >> 8) | ((device_id & 0x0000FF00) << 8) | ((device_id & 0x000000FF) << 24);
	DBG("send picture time %lld\r\n",time);
	data.time[5] = (time & 0x0000000000FF);
	data.time[4] = (time & 0x00000000FF00) >> 8;
	data.time[3] = (time & 0x000000FF0000) >> 16;
	data.time[2] = (time & 0x0000FF000000) >> 24;
	data.time[1] = (time & 0x00FF00000000) >> 32;
	data.time[0] = (time & 0xFF0000000000) >> 40;
	data.picture_id = picture_id;
	data.picture = picture;
	memcpy(sendbuffer, &data, 18);
	memcpy(sendbuffer + 18, data.picture, picture_len);
	struct sockaddr_in servaddr;
	int sockfd2 = socket(PF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(server_port2);
	servaddr.sin_addr.s_addr = inet_addr(server_IP);

	unsigned long ul = 1;
	ioctl(sockfd2, FIONBIO, &ul); //设置为非阻塞模式

	connect(sockfd2, (struct sockaddr *)&servaddr, sizeof(servaddr));

	sleep(3);
	ul = 0;
	ioctl(sockfd2, FIONBIO, &ul); //设置为阻塞模式

	int send_len=send(sockfd2, (char *)sendbuffer, frame_len, 0);
	DBG("send_picture send_len=%d\r\n",send_len);
	if(send_len==-1)
	{
		DBG("send_picture send error:errno=%d\r\n",errno);
		DBG("send_picture send error: %s\r\n",strerror(errno));
		LogDebug(log_sleep, "send_picture send error:errno=%d\r\n",errno);
		LogDebug(log_sleep, "send_picture send error: %s\r\n",strerror(errno));
	}
	/*DBG("send_picture data:\r\n");
	for(int i=0;i<frame_len;i++)
	{
		DBG("0x%02x\r\n",sendbuffer[i]);
	}*/
	shutdown(sockfd2,2);
	close(sockfd2);
	free(sendbuffer);
}
//GPS数据上报信令
void send_gps(uint32_t device_id, uint64_t time, double longitude, double latitude, float v, float heading, uint8_t pos_status)
{
	DBG("function:send_gps\r\n");
	uint8_t *sendbuffer=new uint8_t[42];
	struct upload_gps data;
	DBG("send_gps copy frame_head\r\n");
	data.frame_head = 0x7F7E;
	DBG("send_gps copy len\r\n");
	data.len = 0x27000000;
	DBG("send_gps copy id\r\n");
	data.id = 0x94;
	DBG("send_gps copy device_id\r\n");
	data.device_id = ((device_id & 0xFF000000) >> 24) | ((device_id & 0x00FF0000) >> 8) | ((device_id & 0x0000FF00) << 8) | ((device_id & 0x000000FF) << 24);
	DBG("send_gps copy time\r\n");
	data.time[5] = (time & 0x0000000000FF);
	data.time[4] = (time & 0x00000000FF00) >> 8;
	data.time[3] = (time & 0x000000FF0000) >> 16;
	data.time[2] = (time & 0x0000FF000000) >> 24;
	data.time[1] = (time & 0x00FF00000000) >> 32;
	data.time[0] = (time & 0xFF0000000000) >> 40;
	DBG("send_gps copy longitude\r\n");
	data.longitude = longitude;
	DBG("send_gps copy latitude\r\n");
	data.latitude = latitude;
	DBG("send_gps copy v\r\n");
	data.v = (uint16_t)(v * 100);
	data.v = ((data.v & 0xFF00) >> 8) | ((data.v & 0x00FF) << 8);
	DBG("send_gps copy heading\r\n");
	data.heading = (uint16_t)(heading * 100);
	data.heading = ((data.heading & 0xFF00) >> 8) | ((data.heading & 0x00FF) << 8);
	DBG("send_gps copy version\r\n");
	data.version=Software_Version_second<<8|Software_Version_first;
	DBG("send_gps copy pos_status\r\n");
	data.pos_status=pos_status;
	data.detect_or_save_status=detect_or_save_flag;
	data.aplay_status=aplay_flag;
	memcpy(sendbuffer, &data, 42);
	DBG("send_gps before send:\r\n");
	int send_len=send(sockfd1, (char *)sendbuffer, 42, 0);
	DBG("send_gps send_len=%d\r\n",send_len);
	if(send_len==-1)
	{
		DBG("send_gps send error:errno=%d\r\n",errno);
		DBG("send_gps send error: %s\r\n",strerror(errno));
		LogDebug(log_sleep, "send_gps send error:errno=%d\r\n",errno);
		LogDebug(log_sleep, "send_gps send error: %s\r\n",strerror(errno));
	}
	DBG("send_gps data:\r\n");
	/*for(int i=0;i<40;i++)
	{
		DBG("0x%02x\r\n",sendbuffer[i]);
	}*/
	DBG("0x%02x",sendbuffer[0]);
	for(int i=1;i<42;i++)
	{
		fprintf(stderr," 0x%02x",sendbuffer[i]);
	}
	fprintf(stderr,"\r\n");
	free(sendbuffer);
}
//系统升级回复信令
void send_update(uint32_t device_id, uint8_t update_result)
{
	DBG("function:send_update\r\n");
	uint8_t *sendbuffer=new uint8_t[12];
	struct upload_update data;
	data.frame_head = 0x7F7E;
	data.len = 0x0C000000;
	data.id = 0x80;
	data.device_id = ((device_id & 0xFF000000) >> 24) | ((device_id & 0x00FF0000) >> 8) | ((device_id & 0x0000FF00) << 8) | ((device_id & 0x000000FF) << 24);
	data.update_result = update_result;
	memcpy(sendbuffer, &data, 12);
	int send_len=send(sockfd1, (char *)sendbuffer, 12, 0);
	DBG("send_update send_len=%d\r\n",send_len);
	if(send_len==-1)
	{
		DBG("send_update send error:errno=%d\r\n",errno);
		DBG("send_update send error: %s\r\n",strerror(errno));
		LogDebug(log_sleep, "send_update send error:errno=%d\r\n",errno);
		LogDebug(log_sleep, "send_update send error: %s\r\n",strerror(errno));
	}
	DBG("send_update data:\r\n");
	for(int i=0;i<12;i++)
	{
		DBG("0x%02x\r\n",sendbuffer[i]);
	}
	free(sendbuffer);
}
//GPS数据发送间隔时间上报信令
void send_gps_interval(uint32_t device_id, uint16_t gps_interval)
{
	DBG("function:send_gps_interval\r\n");
	uint8_t *sendbuffer=new uint8_t[13];
	struct upload_gps_interval data;
	data.frame_head = 0x7F7E;
	data.len = 0x0D000000;
	data.id = 0x81;
	data.device_id = ((device_id & 0xFF000000) >> 24) | ((device_id & 0x00FF0000) >> 8) | ((device_id & 0x0000FF00) << 8) | ((device_id & 0x000000FF) << 24);
	data.gps_interval = gps_interval;
	data.gps_interval = ((data.gps_interval & 0xFF00) >> 8) | ((data.gps_interval & 0x00FF) << 8);
	memcpy(sendbuffer, &data, 13);
	int send_len=send(sockfd1, (char *)sendbuffer, 13, 0);
	DBG("send_gps_interval send_len=%d\r\n",send_len);
	if(send_len==-1)
	{
		DBG("send_gps_interval send error:errno=%d\r\n",errno);
		DBG("send_gps_interval send error: %s\r\n",strerror(errno));
		LogDebug(log_sleep, "send_gps_interval send error:errno=%d\r\n",errno);
		LogDebug(log_sleep, "send_gps_interval send error: %s\r\n",strerror(errno));
	}
	DBG("send_gps_interval data:\r\n");
	for(int i=0;i<13;i++)
	{
		DBG("0x%02x\r\n",sendbuffer[i]);
	}
	free(sendbuffer);
}
