#include "string.h"
#include "types.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
//#define uint8_t unsigned char
//#define uint16_t unsigned short int
//#define uint32_t unsigned int
//#define uint64_t unsigned long long int
extern int sockfd1;
#pragma pack(1)
//主动数据帧
//注册信令
struct upload_register
{
	uint16_t frame_head;//帧头
	uint32_t len;//帧长度
	uint8_t id;//信令id
	uint32_t device_id;//设备id
	uint8_t reserve;//保留字节
};
//监测报警信息上报信令
struct upload_alarm
{
	uint16_t frame_head;//帧头
	uint32_t len;//帧长度
	uint8_t id;//信令id
	uint32_t device_id;//设备id
	uint8_t time[6];//unix时间日期
	double longitude;//经度
	double latitude;//纬度
	uint16_t v;//地面速率
	uint16_t heading;//地面航向
	uint8_t status;//报警状态
};
//图片上传信令
struct upload_picture
{
	uint16_t frame_head;//帧头
	uint32_t len;//帧长度
	uint8_t id;//信令id
	uint32_t device_id;//设备id
	uint8_t time[6];//unix时间日期
	uint8_t picture_id;//图片序号
	uint8_t *picture;//图片
};
//GPS数据上报信令
struct upload_gps
{
	uint16_t frame_head;//帧头
	uint32_t len;//帧长度
	uint8_t id;//信令id
	uint32_t device_id;//设备id
	uint8_t time[6];//unix时间日期
	double longitude;//经度
	double latitude;//纬度
	uint16_t v;//地面速率
	uint16_t heading;//地面航向
	uint16_t version;//软件版本号
	uint8_t pos_status;//定位状态
	uint8_t detect_or_save_status;//疲劳检测和视频保存状态
	uint8_t aplay_status;//疲劳检测报警声音状态
};
//被动数据帧
//系统升级回复信令
struct upload_update
{
	uint16_t frame_head;//帧头
	uint32_t len;//帧长度
	uint8_t id;//信令id
	uint32_t device_id;//设备id
	uint8_t update_result;//升级结果
};
//GPS数据发送间隔时间上报信令
struct upload_gps_interval
{
	uint16_t frame_head;//帧头
	uint32_t len;//帧长度
	uint8_t id;//信令id
	uint32_t device_id;//设备id
	uint16_t gps_interval;//间隔时间
};
#pragma pack()
//注册信令
void send_register(uint32_t device_id);
//监测报警信息上报信令
void send_alarm(uint32_t device_id, uint64_t time, double longitude, double latitude, float v, float heading, uint8_t status);
//图片上传信令
void send_picture(uint32_t device_id, uint64_t time, uint8_t picture_id, uint8_t* picture, uint32_t picture_len);
//GPS数据上报信令
void send_gps(uint32_t device_id, uint64_t time, double longitude, double latitude, float v, float heading, uint8_t pos_status);
//系统升级回复信令
void send_update(uint32_t device_id, uint8_t update_result);
//GPS数据发送间隔时间上报信令
void send_gps_interval(uint32_t device_id, uint16_t gps_interval);
