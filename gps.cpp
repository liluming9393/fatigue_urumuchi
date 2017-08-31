#include "gps.h"
#include "upload.h"
#include "log.h"
#include <time.h>
#include <iostream>
using namespace std;
#define BUFSIZE 1024
struct GPS_info gps_info;//建立一个gps_info结构体 将解析后的数据存入
struct tm gps_time;
char gps_analysis_buffer[4096]={0};
uint32_t gps_analysis_buffer_len=0;
uint16_t GPS_interval=1;
uint16_t GPS_interval_count=0;
int sockfd_rtk_input;
bool pos_status_flag=false;//false:single,true:rtk
extern pthread_mutex_t gps_lock;
extern log_st *log_sleep;
void connect_rtk_input_server()
{
	struct sockaddr_in servaddr;
	sockfd_rtk_input = socket(PF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(1240);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	unsigned long ul = 1;
	ioctl(sockfd_rtk_input, FIONBIO, &ul); //设置为非阻塞模式
	connect(sockfd_rtk_input, (struct sockaddr *)&servaddr, sizeof(servaddr));
	ul = 0;
	ioctl(sockfd_rtk_input, FIONBIO, &ul); //设置为阻塞模式
	struct timeval timeout={1,0};//1s
	setsockopt(sockfd_rtk_input,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
	setsockopt(sockfd_rtk_input,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
}
void setTermios(struct termios * pNewtio, int uBaudRate)
{
    bzero(pNewtio, sizeof(struct termios)); /* clear struct for new port settings */
    //8N1
    pNewtio->c_cflag = uBaudRate | CS8 | CREAD | CLOCAL;
    pNewtio->c_iflag = IGNPAR;
    pNewtio->c_oflag = 0;
    pNewtio->c_lflag = 0; //non ICANON
    /*
     initialize all control characters
     default values can be found in /usr/include/termios.h, and
     are given in the comments, but we don't need them here
     */
    pNewtio->c_cc[VINTR] = 0; /* Ctrl-c */
    pNewtio->c_cc[VQUIT] = 0; /* Ctrl-\ */
    pNewtio->c_cc[VERASE] = 0; /* del */
    pNewtio->c_cc[VKILL] = 0; /* @ */
    pNewtio->c_cc[VEOF] = 4; /* Ctrl-d */
    pNewtio->c_cc[VTIME] = 5; /* inter-character timer, timeout VTIME*0.1 */
    pNewtio->c_cc[VMIN] = 0; /* blocking read until VMIN character arrives */
    pNewtio->c_cc[VSWTC] = 0; /* '\0' */
    pNewtio->c_cc[VSTART] = 0; /* Ctrl-q */
    pNewtio->c_cc[VSTOP] = 0; /* Ctrl-s */
    pNewtio->c_cc[VSUSP] = 0; /* Ctrl-z */
    pNewtio->c_cc[VEOL] = 0; /* '\0' */
    pNewtio->c_cc[VREPRINT] = 0; /* Ctrl-r */
    pNewtio->c_cc[VDISCARD] = 0; /* Ctrl-u */
    pNewtio->c_cc[VWERASE] = 0; /* Ctrl-w */
    pNewtio->c_cc[VLNEXT] = 0; /* Ctrl-v */
    pNewtio->c_cc[VEOL2] = 0; /* '\0' */
}
int gps_recv()
{
	DBG("function:gps_recv\r\n");
	connect_rtk_input_server();
	int fd;
	int res;
	char buff[BUFSIZE];
	struct termios oldtio, newtio;
	struct timeval tv;
	//char *dev ="/dev/ttymxc2";
	fd_set rfds;
	int i = 0;

	if ((fd = open(gps_port, O_RDWR | O_NOCTTY))<0){
		DBG("err: can't open serial port!\r\n");
		LogDebug(log_sleep, "err: can't open serial port!\r\n");
		exit(0);
	}

	tcgetattr(fd, &oldtio); /* save current serial port settings */
	setTermios(&newtio, B115200);
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	tv.tv_sec=0;
	tv.tv_usec=100000;

	while (TRUE){
		//printf("wait...\r\n");

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec=0;
		tv.tv_usec=100000;
		if (select(1+fd, &rfds, NULL, NULL, &tv)>0)
		{
			if (FD_ISSET(fd, &rfds))
			{
				//printf("11111\r\n");
				memset (buff, 0, sizeof(buff));
				res=read(fd, buff, BUFSIZE);
				if(res>0)
				{
					int send_len=send(sockfd_rtk_input,buff,res,0);
					//DBG("send raw message to rtkrcv input server send_len=%d\r\n",send_len);
					if(send_len<=0)
					{
						close(sockfd_rtk_input);
						connect_rtk_input_server();
					}
				}
				if(gps_analysis_buffer_len+res>=4096)
				{
					memset(gps_analysis_buffer,0,4096);
					gps_analysis_buffer_len=0;
				}
				memcpy(gps_analysis_buffer+gps_analysis_buffer_len,buff,res);
				//printf("%s",gps_analysis_buffer);
				gps_analysis_buffer_len=gps_analysis_buffer_len+res;
				//cout<<gps_analysis_buffer_len<<endl;
				char analysis[4096]={0};
				memcpy(analysis,gps_analysis_buffer,4096);
				find_str(analysis,"\r\n",4096);
				if(gps_info.status==1)
				{
					GPS_interval_count++;
					DBG("gps_interval_count = %d\r\n",GPS_interval_count);
					DBG("gps_interval = %d\r\n",GPS_interval);
					if(GPS_interval_count==GPS_interval)
					{
						GPS_interval_count=0;
						if(pos_status_flag==false)
						{
							send_gps(Device_ID,gps_info.second,gps_info.Longitude,gps_info.Latitude,gps_info.v,gps_info.heading,0);
						}
					}
					//cout<<"gps send"<<endl;
					//cout<<gps_info.second<<endl;
					gps_info.status=0;
					memset(gps_analysis_buffer,0,sizeof(gps_analysis_buffer));
					gps_analysis_buffer_len=0;

					/*struct tm tm = *localtime((time_t *)&gps_info.second);
					char strTime[100]={0};
					int bufLen=sizeof(strTime);
					strftime(strTime, bufLen - 1, "%Y-%m-%d %H:%M:%S", &tm);
					strTime[bufLen - 1] = '\0';
					//cout << strTime << endl;
					//cout<<"Longitude="<<gps_info.Longitude<<" Latitude="<<gps_info.Latitude<<" time="<<gps_info.second<<endl;
					DBG("gps_time:%s\r\n",strTime);*/
					DBG("GPS result:Longitude=%lf Latitude=%lf time=%lld v=%f heading=%f\r\n",gps_info.Longitude,gps_info.Latitude,gps_info.second,gps_info.v,gps_info.heading);
				}
			}
		}
	}
	return 0;
}

void find_str(char* buffer, char* str, uint32_t buffer_length)
{
	//DBG("function:find_str\r\n");
	uint32_t buffer_length_temp=buffer_length;
	for(int i=0;i<buffer_length_temp-5;i++)
	{
		if(buffer[0]=='$' && buffer[3]=='R' && buffer[4]=='M' && buffer[5]=='C')
		{
			break;
			//buffer++;
		}
		else
		{
			buffer++;
			buffer_length--;
		}
	}
	char* temp = NULL;
	char* gps[15] = { 0 };
	uint8_t n = 0;
	char* start = buffer;
	buffer[buffer_length] = 0x00;
	temp = strstr((char*)buffer, (const char*)str);
	while (temp)
	{
		gps[n] = buffer;
		*temp = 0x00;
		*(temp + 1) = 0x00;
		buffer = temp + strlen((const char*)str);
		if ((buffer - start)>buffer_length) break;
		GPS_analysis((char*)gps[n]);
		n++;
		if (n>15) break;
		temp = strstr(buffer, (const char*)str);
	}
}
uint8_t GPS_analysis(char* gps_buffer)
{
	//DBG("function:GPS_analysis\r\n");
	if(gps_buffer==NULL)
	{
		return 1;
	}
	char* temp[50] = { 0 };
	uint8_t n = 0;
	float heading = 0;
	uint8_t gsv_page = 0;
	char* ptr = gps_buffer;
	char* ptr2 = gps_buffer;
	ptr = strchr((char*)(ptr + 1), ',');
	while (ptr != NULL)
	{
		if ((ptr - ptr2) == 1)
			temp[n] = NULL;
		else temp[n] = ptr2 + 1;
		ptr2 = ptr;
		*ptr2 = 0x00;
		n++;
		ptr = strchr((char*)(ptr + 1), ',');
	}
	ptr = NULL;
	ptr = strchr((char*)(ptr2 + 1), '*');
	if (ptr == NULL) return 0;
	if ((ptr - ptr2) == 1)
		temp[n] = NULL;
	else
	{
		if (ptr != NULL)
			*ptr = 0x00;
		temp[n] = ptr2 + 1;
	}
	if (temp[0]!=NULL && strstr((char*)temp[0], "RMC") != NULL)
	{
		pthread_mutex_lock(&gps_lock);
		if (strstr((char*)temp[2], "V") != NULL)
		{
			gps_info.status = 0x00;
		}
		else if (strstr((char*)temp[2], "A") != NULL)
		{
			gps_info.status = 0x01;
			if (temp[1] != NULL)
				gps_info.time = atof(temp[1]);
			if (temp[3] != NULL)
			{
				double latitude = atof(temp[3]);
				double temp = latitude / 100;
				int temp2 = (int)temp;
				gps_info.Latitude = temp2 + (temp - temp2) * 100 / 60;
			}
			if (temp[5] != NULL)
			{
				double longitude = atof(temp[5]);
				double temp = longitude / 100;
				int temp2 = (int)temp;
				gps_info.Longitude = temp2 + (temp - temp2) * 100 / 60;
			}
			if (temp[7] != NULL)
				gps_info.v = atof(temp[7]);
			if (temp[8] != NULL)
				gps_info.heading = atof(temp[8]);
			else gps_info.heading = 400;
			if (temp[9] != NULL)
			{
				gps_info.date = atof(temp[9]);
				gps_time.tm_sec = gps_info.time % 100;
				gps_time.tm_min = (gps_info.time / 100) % 100;
				gps_time.tm_hour = gps_info.time / 10000;
				gps_time.tm_year = gps_info.date % 100 + 100;
				gps_time.tm_mon = (gps_info.date / 100) % 100 - 1;
				gps_time.tm_mday = (gps_info.date / 10000);
				#ifdef _PC
					gps_info.second = mktime(&gps_time);
				#else
					gps_info.second = mktime(&gps_time)-8*3600;
				#endif
			}
		}
		pthread_mutex_unlock(&gps_lock);
	}
	return 1;
}
