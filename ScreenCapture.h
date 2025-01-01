#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H
#include <QPixmap>

// OpenCV Headers
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class ScreenCapture {
public:
    ScreenCapture(int model,int width,int height);
    // 屏幕捕获函数（基于 DirectX 11）
    cv::Mat runCapture();
    void reviseParameter(int model,int width,int height);
private:
  int model;
  int width;
  int height;
};



#endif //SCREENCAPTURE_H
