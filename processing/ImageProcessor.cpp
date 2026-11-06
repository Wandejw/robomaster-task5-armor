#include "ImageProcessor.h"
#include <opencv2/imgproc.hpp>

void preprocess_image(const cv::Mat& image, cv::Mat& mask)
{
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    const cv::Scalar lowerBound(80, 30, 150);
    const cv::Scalar upperBound(130, 255, 255);

    cv::inRange(hsv, lowerBound, upperBound, mask);
    cv::Mat redLow, redHigh;
    cv::inRange(hsv, cv::Scalar(0, 30, 150), cv::Scalar(20, 255, 255), redLow);
    cv::inRange(hsv, cv::Scalar(170, 30, 150), cv::Scalar(179, 255, 255), redHigh);
    cv::Mat redMask;
    cv::bitwise_or(redLow, redHigh, redMask);
    // 先膨胀再腐蚀：尝试连接竖直灯条中的短间断。
    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5));
    // 创建一个 1x5 的矩形结构元素，用于膨胀和腐蚀操作。
    cv::Mat dilated;
    cv::dilate(mask, dilated, kernel);
    //腐蚀操作，使用相同的结构元素
    cv::erode(dilated, mask, kernel);
    cv::morphologyEx(redMask, redMask, cv::MORPH_CLOSE, kernel);
    cv::Mat blueMask = mask.clone();
    mask.setTo(128, redMask);
    mask.setTo(255, blueMask);
    // 255 表示蓝色候选，128 表示红色候选，0 表示背景。
}
