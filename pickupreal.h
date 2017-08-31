#ifndef __PICKUPREAL_H__
#define __PICKUPREAL_H__

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

class CPickUpReal
{
private:
    Point pntLastCenter;
    int lastSize;
public:
    CPickUpReal();
    ~CPickUpReal();
    bool Excute(const vector<Rect> &src, Rect &dst);
    Point CalcRectCenter(const Rect &rct);
};

#endif
