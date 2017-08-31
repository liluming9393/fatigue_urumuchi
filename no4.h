#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include "pickupreal.h"
#include "capture.h"
#include "cv.h"
#include "highgui.h"
#include "unistd.h"
#include "types.h"
#include <sys/time.h>
int detect();
float MeasureOpenValue(const Mat &src, int faceHeight, int blockSize, int constantSubtract, float maxVal = 0.0f, float minVal = 0.0f, Mat *imgDebug = NULL);
bool computer_sleepy(vector<struct frame>window_frame);
void study_stage(vector<struct frame>window_frame,float* avg_left,float* avg_right);
