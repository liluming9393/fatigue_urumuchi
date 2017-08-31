/*
 * process.cpp
 *
 *  Created on: 2017年1月11日
 *      Author: liluming
 */
#include "process.h"
#include "types.h"
#include "gps.h"
#include "upload.h"
using namespace std;
using namespace cv;
extern struct GPS_info gps_info;//建立一个gps_info结构体 将解析后的数据存入
extern pthread_mutex_t gps_lock;
void alarm(Mat img)
{
	//cout<<"Alarm!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	DBG("function:alarm!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\r\n");
	pthread_mutex_lock(&gps_lock);
	uint64_t alarm_time=gps_info.second;
	double gps_longitude=gps_info.Longitude;
	double gps_latitude=gps_info.Latitude;
	float gps_v=gps_info.v;
	float gps_heading=gps_info.heading;
	pthread_mutex_unlock(&gps_lock);
	/*cout<<"Alarm time="<<alarm_time<<endl;
	cout<<"gps longitude"<<gps_longitude<<endl;
	cout<<"gps latitude"<<gps_latitude<<endl;
	cout<<"gps v"<<gps_v<<endl;
	cout<<"gps heading"<<gps_heading<<endl;*/
	//send_alarm(Device_ID,alarm_time,gps_info.Longitude,gps_info.Latitude,gps_info.v,gps_info.heading,1);
	send_alarm(Device_ID,alarm_time,gps_longitude,gps_latitude,gps_v,gps_heading,1);
	IplImage qImg;
	qImg = IplImage(img); // cv::Mat -> IplImage
	cvSaveImage("1.jpg", &qImg);
	FILE *stream;
	stream = fopen("1.jpg", "r");
	fseek(stream, 0, SEEK_END);     //定位到文件末
	int picture_len;
	picture_len = ftell(stream);       //文件长度
	fclose(stream);
	stream = NULL;
	stream = fopen("1.jpg", "rb");
	uint8_t ch;
	uint8_t *picture=new uint8_t[picture_len];
	for (int i = 0; i < picture_len;i++)
	{
		/* read a char from the file */
		ch = fgetc(stream);
		picture[i] = ch;
		/* display the character */
	}
	fclose(stream);
	stream = NULL;
	uint8_t picture_id=1;
	send_picture(Device_ID,alarm_time,picture_id,picture,picture_len);
}



