#include "pickupreal.h"

CPickUpReal::CPickUpReal()
:lastSize(0)
{
}

CPickUpReal::~CPickUpReal()
{
}

bool CPickUpReal::Excute(const vector<Rect> &src, Rect &dst)
{
    double resizeRatio;
    double moveRatio;
    double changeRatio;
    double changeRatioMin;
    Point pntCenter;

    int realIdx = -1;
    if (src.size() == 1) {
        realIdx = 0;
        pntLastCenter = CalcRectCenter(src[0]);
        lastSize = src[0].width;
    }
    else if (src.size() > 1 && lastSize != 0) {
        changeRatioMin = 100;
        for (int i = 0; i < src.size(); ++i) {
            resizeRatio = fabs((double)src[i].width / lastSize - 1);
            pntCenter = CalcRectCenter(src[i]);
            moveRatio = sqrt(pow(pntCenter.x - pntLastCenter.x, 2) + pow(pntCenter.y - pntLastCenter.y, 2)) / lastSize;
            changeRatio = (moveRatio + resizeRatio) / 2;
            if (changeRatioMin > changeRatio) {
                changeRatioMin = changeRatio;
                realIdx = i;
            }
        }
    }

    if (realIdx == -1) {
        return false;
    }
    else {
        dst = src[realIdx];
        return true;
    }
}

Point CPickUpReal::CalcRectCenter(const Rect &rct)
{
    Point pnt;
    pnt.x = rct.x + rct.width / 2;
    pnt.y = rct.y + rct.height / 2;
    return pnt;
}
