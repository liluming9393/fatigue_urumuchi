/*
 * savereal.cpp
 *
 *  Created on: 2017年4月4日
 *      Author: alicia
 */
#include "savecamera.h"
#include "types.h"
#include "log.h"
#include <fstream>
#include <sstream>
VideoWriter outputVideo;
extern log_st *log_sleep;
string savevideo_name;
using namespace std;
int saveCamera () {
	// 视频保存路径
	uint32_t savecamera_count=0;
	ifstream in("SaveCamera_count.txt");
	if(!in.is_open())
	{
		DBG("Open SaveCamera_count.txt read failed\r\n");
		LogDebug(log_sleep, "Open SaveCamera_count.txt read failed\r\n");
	}
	else
	{
		in>>savecamera_count;
		DBG("read SaveCamera_count : %d\r\n",savecamera_count);
	}
	in.close();
	in.clear();
	savecamera_count++;
	ofstream out("SaveCamera_count.txt");
	if(!out.is_open())
	{
		DBG("Open SaveCamera_count.txt write failed\r\n");
		LogDebug(log_sleep, "Open SaveCamera_count.txt write failed\r\n");
	}
	else
	{
		DBG("write SaveCamera_count : %d\r\n",savecamera_count);
		out<<savecamera_count;
	}
	out.close();
	out.clear();
	savevideo_name.clear();
	stringstream ss;
	ss<<savecamera_count;
	ss>>savevideo_name;
	savevideo_name=save_path+savevideo_name;
	savevideo_name+=".avi";
	//cv::String videoPath = "./h246_fps6_2.avi";
	cv::String videoPath = savevideo_name;
	/*
	// 打开摄像头
	cv::VideoCapture capture0(0);
	//capture0.open(0);
	if (!capture0.isOpened()) {
		cout << "fail to open camera!" << endl;
		return -1;
	}
	else {
		cout << "open camera successfully!" << endl;
	}
	// 获得当前摄像头的视频信息
	double fps = capture0.get(CV_CAP_PROP_FPS);
	cout << "fps的值为：" << fps << endl;
	cv::Size size = cv::Size((int) capture0.get(CV_CAP_PROP_FRAME_WIDTH),
							(int) capture0.get(CV_CAP_PROP_FRAME_HEIGHT));
	 */
	#ifdef _PC
		int fourcc = CV_FOURCC('M','J','P','G');
		//int fourcc = CV_FOURCC('H','2','6','4');
	#else
		int fourcc = CV_FOURCC('H','2','6','4');
	#endif
	//int fourcc = CV_FOURCC('M','P','4','2');
	//int fourcc = CV_FOURCC('X','2','6','4');
	//int fourcc = CV_FOURCC('H','2','6','4');
	//int fourcc = CV_FOURCC('M','P','E','G');
	//int fourcc = CV_FOURCC('X','V','I','D');
	//int fourcc = CV_FOURCC('D','I','V','X');
	//int fourcc = CV_FOURCC('M','J','P','G');
	//int fourcc = CV_FOURCC('P','I','M','1');
	//int fourcc = CV_FOURCC('F','L','V','1');
	int fps = 6.0;
	outputVideo.open(videoPath,fourcc,fps,Size(640,480),true);
	if (!outputVideo.isOpened()) {
		DBG("savecamera : fail to open video!\r\n");
		LogDebug(log_sleep, "savecamera : fail to open video!\r\n");
		//cout << "fail to open video!" << endl;
		return -1;
	}
	else {
		DBG("savecamera : open video successfully!\r\n");
		//cout << "open video successfully!" << endl;
	}
	/*
	cv::Mat frameImage;
	int count = 0;
	while (1) {
		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv,&tz);
		cout<<tv.tv_sec<<endl;
		 //　读取当前帧
		capture0.operator >> (frameImage);
		if (frameImage.empty())
		{
			cout << "fail to save!" << endl;
			continue;
		}
		 ++count;
		 cout << count << endl;
		 if(count==1000)
		 {
			 return 0;
		 }
		 //　输出当前帧
		 //cv::imshow("output", frameImage);
		 //　保存当前帧
		 outputVideo << frameImage;
		 if (count == 500) {
			 cout << "finish to save!" << endl;
			 break;
		 }
		 //waitKey(1000/30);
	}
	capture0.release();
	 */
	return 0;
}
