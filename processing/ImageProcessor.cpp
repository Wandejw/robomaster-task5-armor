#include "ImageProcessor.h"
#include <opencv2/imgproc.hpp>

void preprocess_image(const cv::Mat& image, cv::Mat& mask)
{
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    const cv::Scalar lowerBound(80, 30, 150);
    const cv::Scalar upperBound(130, 255, 255);

    cv::inRange(hsv, lowerBound, upperBound, mask);
    // 先膨胀再腐蚀：尝试连接竖直灯条中的短间断。
    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5));
    // 创建一个 1x5 的矩形结构元素，用于膨胀和腐蚀操作;
    cv::Mat dilated;
    cv::dilate(mask, dilated, kernel);
    //腐蚀操作，使用相同的结构元素
    cv::erode(dilated, mask, kernel);
    // 现在 mask 中的白色区域应该是蓝色灯条的候选区域，黑色区域是背景。
}
