#include "ImageProcessor.h"
#include <opencv2/imgproc.hpp>

void preprocess_image(const cv::Mat& image, cv::Mat& mask)
{
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    const cv::Scalar lowerBound(80, 30, 150);
    const cv::Scalar upperBound(130, 255, 255);

    cv::inRange(hsv, lowerBound, upperBound, mask);  
    // 下一步：筛选 H、S、V 范围，生成 mask
}
