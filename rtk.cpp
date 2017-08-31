/*
 * rtk.cpp
 *
 *  Created on: 2017年4月6日
 *      Author: root
 */
#include "rtk.h"
#include "upload.h"
#include "log.h"
char rtk_recv_buffer[1024]={0};
char rtk_recv_anlysis_buffer[4096]={0};
int rtk_recv_anlysis_buffer_len=0;
int sockfd_rtk_result;
struct RTK_info rtk_info;
struct tm rtk_time;
int rtk_error_count=0;
bool time_set_flag=false;//if time set correct then this flat is true
int rtk_analysis_fail_count=0;
extern bool pos_status_flag;//false:single,true:rtk
extern log_st *log_sleep;
uint8_t Default_CFG[21]={0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x1B, 0x9A};
uint8_t Default_CFG_len=21;
uint8_t GGA_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x24};
uint8_t GGA_disable_len=16;
uint8_t GLL_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B};
uint8_t GLL_disable_len=16;
uint8_t GSA_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32};
uint8_t GSA_disable_len=16;
uint8_t GSV_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39};
uint8_t GSV_disable_len=16;
uint8_t RMC_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x40};
uint8_t RMC_disable_len=16;
uint8_t VTG_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47};
uint8_t VTG_disable_len=16;
uint8_t ZDA_disable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x5C};
uint8_t ZDA_disable_len=16;
uint8_t RAWX_enable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x02, 0x15, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x2B, 0x5A};
uint8_t RAWX_enable_len=16;
uint8_t SFRBX_enable[16]={0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x02, 0x13, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x29, 0x4C};
uint8_t SFRBX_enable_len=16;
uint8_t CFG_PRT[28]={0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7E};
uint8_t CFG_PRT_len=28;

void rtk_init()
{
	DBG("rtk_init\r\n");
	int fd_serial_rtk;
	struct termios oldtio, newtio;
	struct timeval tv;
	int res=0;
	//char *dev ="/dev/ttymxc2";
	if ((fd_serial_rtk = open(rtk_port, O_RDWR | O_NOCTTY))<0){
		//printf("err: can't open serial port!\r\n");
		DBG("err: can't open serial port!\r\n");
		exit(0);
	}
	tcgetattr(fd_serial_rtk, &oldtio); /* save current serial port settings */
	setTermios_rtk(&newtio, B9600);
	tcflush(fd_serial_rtk, TCIFLUSH);
	tcsetattr(fd_serial_rtk, TCSANOW, &newtio);
	tv.tv_sec=30;
	tv.tv_usec=0;
	usleep(20000);
	res=write(fd_serial_rtk,Default_CFG,Default_CFG_len);
	DBG("write Default_CFG return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,GGA_disable,GGA_disable_len);
	DBG("write GGA_disable return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,GLL_disable,GLL_disable_len);
	DBG("write GLL_disable return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,GSA_disable,GSA_disable_len);
	DBG("write GSA_disable return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,GSV_disable,GSV_disable_len);
	DBG("write GSV_disable return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,RMC_disable,RMC_disable_len);
	DBG("write RMC_disable return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,VTG_disable,VTG_disable_len);
	DBG("write VTG_disable return = %d\r\n",res);
	usleep(20000);
	res=write(fd_serial_rtk,ZDA_disable,ZDA_disable_len);
	DBG("write ZDA_disable return = %d\r\n",res);
	usleep(20000);
	write(fd_serial_rtk,RAWX_enable,RAWX_enable_len);
	DBG("write RAWX_enable return = %d\r\n",res);
	usleep(20000);
	write(fd_serial_rtk,SFRBX_enable,SFRBX_enable_len);
	DBG("write SFRBX_enable return = %d\r\n",res);
	usleep(50000);
	write(fd_serial_rtk,CFG_PRT,CFG_PRT_len);
	DBG("write CFG_PRT return = %d\r\n",res);
	usleep(50000);
	close(fd_serial_rtk);
	cout<<"close serial rtk"<<endl;
}
void setTermios_rtk(struct termios * pNewtio, int uBaudRate)
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
void connect_rtk_result_server()
{
	struct sockaddr_in servaddr;
	sockfd_rtk_result = socket(PF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(1230);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	unsigned long ul = 1;
	ioctl(sockfd_rtk_result, FIONBIO, &ul); //设置为非阻塞模式
	connect(sockfd_rtk_result, (struct sockaddr *)&servaddr, sizeof(servaddr));
	ul = 0;
	ioctl(sockfd_rtk_result, FIONBIO, &ul); //设置为阻塞模式
	struct timeval timeout={1,0};//1s
	setsockopt(sockfd_rtk_result,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
	setsockopt(sockfd_rtk_result,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
}

void rtk_recv_analysis()
{
	int rtk_recv_len=recv(sockfd_rtk_result,rtk_recv_buffer,1024,0);
	DBG("rtk_recv_analysis rtk_recv_len=%d\r\n",rtk_recv_len);
	if(rtk_recv_len<=0)
	{
		rtk_error_count++;
		//sleep(1);
		if(rtk_error_count==30)
		{
			rtk_error_count=0;
			DBG("restart rtkrcv\r\n");
			LogDebug(log_sleep, "restart rtkrcv\r\n");
			DBG("kill rtkrcv single\r\n");
			system("kill -9 `pidof rtkrcv`");
			sleep(1);
			DBG("start rtkrcv rtk\r\n");
			if(time_set_flag==false)
			{
				system("./rtkrcv -s -o rtkrcv_single.conf &");
			}
			else
			{
				#ifdef _PC
					system("./rtkrcv -s -o rtkrcv_single.conf &");
				#else
					system("./rtkrcv -s -o rtkrcv_rtk.conf &");
				#endif
			}
			sleep(5);
			//cout<<"relink rtk server"<<endl;
			DBG("relink rtk server\r\n");
			sleep(1);
			close(sockfd_rtk_result);
			connect_rtk_result_server();
		}
	}
	else if(rtk_recv_len>0)
	{
		rtk_error_count=0;
		if(rtk_recv_anlysis_buffer_len+rtk_recv_len>=4096)
		{
			memset(rtk_recv_anlysis_buffer,0,4096);
			rtk_recv_anlysis_buffer_len=0;
		}
		memcpy(rtk_recv_anlysis_buffer+rtk_recv_anlysis_buffer_len,rtk_recv_buffer,rtk_recv_len);
		rtk_recv_anlysis_buffer_len=rtk_recv_anlysis_buffer_len+rtk_recv_len;
		char analysis[4096]={0};
		memcpy(analysis,rtk_recv_anlysis_buffer,4096);
		DBG("rtk analysis\r\n");
		find_str_rtk(analysis,"\r\n",4096);
		if(rtk_info.status==1)
		{
			DBG("send rtk result\r\n");
			rtk_analysis_fail_count=0;
			pos_status_flag=true;
			send_gps(Device_ID,rtk_info.second,rtk_info.Longitude,rtk_info.Latitude,rtk_info.v,rtk_info.heading,1);
			rtk_info.status=0;
			memset(rtk_recv_anlysis_buffer,0,sizeof(rtk_recv_anlysis_buffer));
			rtk_recv_anlysis_buffer_len=0;
			//cout<<"Longitude="<<rtk_info.Longitude<<" Latitude="<<rtk_info.Latitude<<" time="<<rtk_info.second<<" heading="<<rtk_info.heading<<" v="<<rtk_info.v<<endl;
			DBG("RTKRCV result:Longitude=%lf Latitude=%lf time=%lld heading=%lf v=%lf\r\n",rtk_info.Longitude,rtk_info.Latitude,rtk_info.second,rtk_info.heading,rtk_info.v);
		}
		else
		{
			rtk_analysis_fail_count++;
			if(rtk_analysis_fail_count==2)
			{
				DBG("change to send single result\r\n");
				rtk_analysis_fail_count=0;
				pos_status_flag=false;
			}

		}
	}
}
void find_str_rtk(char* buffer, char* str, uint32_t buffer_lengh)
{
	//DBG("function:find_str\r\n");
	char* temp = NULL;
	char* gps[15] = { 0 };
	uint8_t n = 0;
	char* start = buffer;
	buffer[buffer_lengh] = 0x00;
	temp = strstr((char*)buffer, (const char*)str);
	while (temp)
	{
		gps[n] = buffer;
		*temp = 0x00;
		*(temp + 1) = 0x00;
		buffer = temp + strlen((const char*)str);
		if ((buffer - start)>buffer_lengh) break;
		GPS_analysis_rtk((char*)gps[n]);
		n++;
		if (n>15) break;
		temp = strstr(buffer, (const char*)str);
	}
}
uint8_t GPS_analysis_rtk(char* gps_buffer)
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
		if (strstr((char*)temp[2], "V") != NULL)
		{
			rtk_info.status = 0x00;
		}
		else if (strstr((char*)temp[2], "A") != NULL)
		{
			rtk_info.status = 0x01;
			if (temp[1] != NULL)
				rtk_info.time = atof(temp[1]);
			if (temp[3] != NULL)
			{
				double latitude = atof(temp[3]);
				double temp = latitude / 100;
				int temp2 = (int)temp;
				rtk_info.Latitude = temp2 + (temp - temp2) * 100 / 60;
			}
			if (temp[5] != NULL)
			{
				double longitude = atof(temp[5]);
				double temp = longitude / 100;
				int temp2 = (int)temp;
				rtk_info.Longitude = temp2 + (temp - temp2) * 100 / 60;
			}
			if (temp[7] != NULL)
				rtk_info.v = atof(temp[7]);
			if (temp[8] != NULL)
				rtk_info.heading = atof(temp[8]);
			else rtk_info.heading = 400;
			if (temp[9] != NULL)
			{
				rtk_info.date = atof(temp[9]);
				rtk_time.tm_sec = rtk_info.time % 100;
				rtk_time.tm_min = (rtk_info.time / 100) % 100;
				rtk_time.tm_hour = rtk_info.time / 10000;
				rtk_time.tm_year = rtk_info.date % 100 + 100;
				rtk_time.tm_mon = (rtk_info.date / 100) % 100 - 1;
				rtk_time.tm_mday = (rtk_info.date / 10000);
				#ifdef _PC
					rtk_info.second = mktime(&rtk_time);
				#else
					rtk_info.second = mktime(&rtk_time)-8*3600;
				#endif
				if(time_set_flag==false)
				{
					time_set_flag=true;
					struct timeval tv;
					struct timezone tz;
					gettimeofday(&tv,&tz);
					tv.tv_sec=rtk_info.second+8*3600;
					DBG("set time\r\n");
					DBG("%lld\r\n",rtk_info.second);
					DBG("%ld\r\n",tv.tv_sec);
					settimeofday(&tv,NULL);
					sleep(1);
					DBG("kill rtkrcv single\r\n");
					system("kill -9 `pidof rtkrcv`");
					sleep(1);
					DBG("start rtkrcv rtk\r\n");
					#ifdef _PC
						system("./rtkrcv -s -o rtkrcv_single.conf &");
					#else
						system("./rtkrcv -s -o rtkrcv_rtk.conf &");
					#endif
				}
			}
		}
	}
	return 1;
}
