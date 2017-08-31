#include "no4.h"
#include "process.h"

#define SLEEPY_VALUE 20
#define LEFT_EYE_VALUE 0.3
#define RIGHT_EYE_VALUE 0.5
#define WINDOWS_WIDTH 80

int sleep_value=0;
float left_eye_value=0;
float right_eye_value=0;
int window_width=0;
int detect_time=0;

bool alarm_flag=false;
using namespace cv;
using namespace std;
Mat img;
uint8_t alarm_count=0;
uint8_t detect_count=0;
bool detect_flag=true;//检测报警开关标志，true开，false关
extern bool detect_or_save_flag;
extern bool aplay_flag;
int detect()
{
	string strHomeDir;
	string strDataDir;
#ifdef _PC
	strHomeDir = "/home/liluming/opencv-3.0.0/data";
	strDataDir = "/home/gyz/data/";
#else
	strHomeDir = "/usr/share/OpenCV";
	strDataDir = "/home/gyz/data/";
#endif
	img=Mat(480,640,CV_8UC3);
	init_();
	mainloop();
    
    if (img.empty()) {
        return 0;
    }

    CascadeClassifier cc(strHomeDir + "/haarcascades/haarcascade_frontalface_alt2.xml");
    CascadeClassifier ccEye[2];
    ccEye[0].load(strHomeDir + "/haarcascades/haarcascade_righteye_2splits.xml");
    ccEye[1].load(strHomeDir + "/haarcascades/haarcascade_lefteye_2splits.xml");

    int constantSubtract = 8;
    int blockSize0 = 17;
    int blockSize = blockSize0 * 2 + 1;

    Point pntLastFaceCenter(0, 0);
    Point pntFaceCenter(0, 0);
    Rect rctEye;
    int i;

    Mat imgGray;
    Mat imgGraySmall;
    Mat imgGraySmall2;
    Mat imgFace;
    Mat imgMouth;
    Mat imgEyeRoi[2];
    Mat imgBinary;
    Mat imgEye[2];

    CvMat img2=img;
    vector<Rect> vctFaces;
    Rect rctMouth;

    CPickUpReal purFace;
    CPickUpReal purEye[2];
    
    bool out;
    struct frame frame_buffer={0,0,0,true};
    float avg_left_eye,avg_right_eye=0;
    vector<struct frame>window_frame;
    bool running = true;
    do {
    	if(detect_or_save_flag==false)
    	{
			detect_count++;
			if(detect_count==detect_time)
			{
				detect_count=0;
				alarm_count=0;
			}
			if(window_frame.size()>window_width){
				out=computer_sleepy(window_frame);
				window_frame.erase(window_frame.begin());
				//cout<<"vector has "<<window_width<<endl;
				//cout<<out<<endl;
				DBG("vector has %d\r\n",window_width);
				DBG("%d\r\n",out);
				if(out==true)
				{
					//cout<<"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"<<endl;
					DBG("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ\r\n");
					if(alarm_flag==false)
					{
						alarm_count++;
						if(alarm_count==1)
						{
							detect_count=0;
						}
						if(detect_flag  && aplay_flag)
						{
							system("aplay alarm.wav &");
						}
						//alarm_sound();
						if(alarm_count==3)
						{
							alarm_count=0;
							if(detect_flag)
							{
								alarm(img);
							}
						}
						alarm_flag=true;
					}

					CvFont font;
					cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC,1,1,0,2);
					cvPutText((CvArr*)&img2,"SLEEPY",cvPoint(200,100), &font,cvScalar(0,0,255));
				}
				else
				{
					alarm_flag=false;
				}
			}
			/*else if(window_frame.size()<window_width){
				study_stage(window_frame,&avg_left_eye,&avg_right_eye);
				cout<<"****************study***************"<<endl;
			}*/
			else DBG("-------------------------------\r\n");//cout<<"------------------------------------"<<endl;

			cvtColor(img, imgGray, CV_RGB2GRAY);
			pyrDown(imgGray, imgGraySmall);
		  // pyrDown(imgGraySmall2,imgGraySmall);


			//cvWaitKey(5);
			usleep(5000);
			// detect face
			cc.detectMultiScale(imgGraySmall, vctFaces, 1.5 , 3, 0, Size(100, 100));
			Rect face;

			// pickup real face
			Rect rctFace;
			if (!purFace.Excute(vctFaces, rctFace)) {
				CvFont font;
				cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC,1,1,0,2);
				cvPutText((CvArr*)&img2,"CAUTION!!!",cvPoint(200,100), &font,cvScalar(255,0,255));
				//imshow("1",img);
				mainloop();
				//frame_buffer={0,0,0,true};
				if (img.empty()) {
					running = false;
				}
				//cout<<"Don't Detect"<<endl;
				DBG("Don't Detect\r\n");
			   // window_frame.push_back(frame_buffer);
				continue;
			}

			//cout<<"Detect is OK"<<endl;
			//cout<<avg_left_eye<<"  "<<avg_right_eye<<endl;
			DBG("Detect is OK\r\n");
			//DBG("%d %d\r\n",avg_left_eye,avg_right_eye);
			rctFace.height *= 1.2;
			if (rctFace.height > imgGraySmall.rows - rctFace.y) {
				rctFace.height = imgGraySmall.rows - rctFace.y;
			}

			imgGraySmall(rctFace).copyTo(imgFace);
			face.x=rctFace.x*2;
			face.y=rctFace.y*2;
			face.height=rctFace.height*2;
			face.width=rctFace.width*2;
		   // rectangle(img,face,Scalar(0,255,255),3,8,0);

			rctMouth.x = imgFace.cols / 6;
			rctMouth.y = imgFace.rows / 2;
			rctMouth.width = imgFace.cols * 2 / 3;
			rctMouth.height = imgFace.rows - rctMouth.y;
			imgFace(rctMouth).copyTo(imgMouth);

			Rect mouth;
			mouth.x=face.x+rctMouth.x*2;
			mouth.y=face.y+rctMouth.y*2;
			mouth.height=rctMouth.height*1.5;
			mouth.width=rctMouth.width*2;
		  //  rectangle(img,mouth,Scalar(255,0,255),3,8,0);
		   // imshow("2",imgFace);

			// measure mouth open value
			Mat imgmouthDebug;
			float mouthOpenValue = MeasureOpenValue(imgMouth, imgFace.rows, 35, 13, 0.19f, 0.02213f,&imgmouthDebug);
			//cout << "mouth open value:" << mouthOpenValue << endl;
			DBG("mouth open value: %lf\r\n",mouthOpenValue);
			rctEye.width = rctFace.width;
			rctEye.height = rctFace.height;
			rctEye.y = rctFace.y * 2;
			for (i = 0; i < 2; ++i) {
				rctEye.x = rctFace.x * 2 + i * rctEye.width;
				imgGray(rctEye).copyTo(imgEyeRoi[i]);
			}
			rectangle(imgGraySmall,rctEye,Scalar(0,0,255),3,8,0);
			float eyeOpenValue[2] = {0};
			Mat imgDebug[2];
			for (i = 0; i < 2; ++i) {
				vector<Rect> vctEyes;
				// detect eye
				ccEye[i].detectMultiScale(imgEyeRoi[i], vctEyes, 1.1);
				if (vctEyes.size() == 0) {
					eyeOpenValue[i] = 0;
				}
				// pickup real eyes
				else if (purEye[i].Excute(vctEyes, rctEye)) {
					rctEye.height /= 2;
					rctEye.y += rctEye.height;
					imgEyeRoi[i](rctEye).copyTo(imgEye[i]);
					// measure eye open value
					eyeOpenValue[i] = MeasureOpenValue(imgEye[i], rctFace.height * 2, blockSize, constantSubtract, 0.04659f, 0.02331f, &imgDebug[i]);
				}
			}

			//imshow("1",img);
		   // imshow("2",imgEye[0]);
			//cout << "Eye0 open value" << eyeOpenValue[0] << endl;
			//cout << "Eye1 open value" << eyeOpenValue[1] << endl;
			DBG("Eye0 open value %lf\r\n",eyeOpenValue[0]);
			DBG("Eye1 open value %lf\r\n",eyeOpenValue[1]);
			bool is_sleepy;
			if(eyeOpenValue[0]<left_eye_value && eyeOpenValue[1]<right_eye_value){
				is_sleepy=true;
			}
			else is_sleepy=false;
			frame_buffer={eyeOpenValue[0],eyeOpenValue[1],mouthOpenValue,is_sleepy};
			window_frame.push_back(frame_buffer);
		}
		 /*   if(!imgDebug[0].empty()) {
				imshow("debug0", imgDebug[0]);
			}
			if(!imgDebug[1].empty()) {
				imshow("debug1", imgDebug[1]);
			}
			if(!imgmouthDebug.empty()){
				imshow("mouthdebug",imgmouthDebug);
			}*/



			//cout << "blockSize=" << blockSize << " constantSubtract=" << constantSubtract << endl;
		   /* IplImage qimg;
			qimg=IplImage(img);
			cvSaveImage("1.jpg",&qimg);
			imshow("Image", img);
			while(1);
			int key = waitKey() & 0x00FF;
			switch (key) {
				case 27:
					running = false;
					break;
				case 'b':
					blockSize0--;
					if (blockSize0 < 1) {
						blockSize0 = 1;
					}
					blockSize = blockSize0 * 2 + 1;
					break;
				case 'B':
					blockSize0++;
					blockSize = blockSize0 * 2 + 1;
					break;
				case 'c':
					constantSubtract--;
					break;
				case 'C':
					constantSubtract++;
					break;
				case 'n':*/
            	mainloop();
                if (img.empty()) {
                    running = false;
                }
      /*          break;
            default:
            ;    
        }*/
    } while (running);

    return 0;
}

float MeasureOpenValue(const Mat &src, int faceHeight, int blockSize, int constantSubtract, float maxVal, float minVal,  Mat *imgDebug)
{
    Mat imgBinary;

    adaptiveThreshold(src, imgBinary, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, blockSize, constantSubtract);

    Point pntCenter(0, 0);
    int sMax = 0;
    int i;
    for (i = 0; i < imgBinary.rows; ++i) {
        Scalar s = sum(imgBinary.row(i));
        if (sMax < s[0]) {
            sMax = s[0];
            pntCenter.y = i;
        }
    }

    for (i = 0; i < imgBinary.cols; ++i) {
        if (imgBinary.data[pntCenter.y * imgBinary.step + i] == 255) {
            pntCenter.x += i;
        }
    }
    pntCenter.x /= (sMax / 255);

    int startIdx = pntCenter.x - imgBinary.cols / 7;
    if (startIdx < 0) {
        startIdx = 0;
    }
    int endIdx = pntCenter.x + imgBinary.cols / 7;
    if (endIdx >= imgBinary.cols) {
        endIdx = imgBinary.cols - 1;
    }
    startIdx += pntCenter.y * imgBinary.step;
    endIdx += pntCenter.y * imgBinary.step;
    for (i = startIdx; i <= endIdx; ++i) {
        imgBinary.data[i] = 255;
    }

    floodFill(imgBinary, pntCenter, CV_RGB(128, 128, 128));

    if(imgDebug != NULL)
    {
        imgBinary.copyTo(*imgDebug);   
    }

    int top = 0;
    while (1) {
        if (imgBinary.data[top * imgBinary.step + pntCenter.x] == 128) {
            break;
        }
        top++;
    }
    int bottom = imgBinary.rows - 1;
    while (1) {
        if (imgBinary.data[bottom * imgBinary.step + pntCenter.x] == 128) {
            break;
        }
        bottom--;
    }
    float openValue = bottom - top;
    openValue /= faceHeight;

    if (maxVal != 0.0f) {
        openValue = (openValue - minVal) / (maxVal - minVal);
        if (openValue < 0.0f) {
            openValue = 0.0f;
        }
        else if (openValue > 1.0f) {
            openValue = 1.0f;
        }
    }

    return openValue;
}
bool computer_sleepy(vector<struct frame>window_frame){
	int sum_sleepy=0;
	for(int frame_index=0;frame_index<window_frame.size();frame_index++)
	{
		if(window_frame[frame_index].is_sleepy)sum_sleepy++;
	}
	if(sum_sleepy>sleep_value) return true;
	else return false;
}
void study_stage(vector<struct frame>window_frame,float* avg_left,float* avg_right){
	float sum_left_eye=0,sum_right_eye=0;
	for(int frame_index=0;frame_index<window_frame.size();frame_index++)
	{
		sum_left_eye+=window_frame[frame_index].left_eye;
		sum_right_eye+=window_frame[frame_index].right_eye;
	}
	*avg_left=sum_left_eye/window_frame.size();
	*avg_right=sum_right_eye/window_frame.size();
}

