#include "LightDetector.h"
#include <opencv2/imgproc.hpp>
#include <vector>
#include <algorithm>

void light_detector(const cv::Mat& mask, cv::Mat& lightMask,std::vector<LightBar>& lights)
// 识别灯条轮廓并绘制到 lightMask 上，同时将识别到的灯条信息存储到 lights 向量中
{
    lights.clear();
    std::vector<std::vector<cv::Point>> contours;

    cv::findContours(mask,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
//REER形式忽视内部轮廓，CHAIN_APPROX_SIMPLE形式压缩轮廓点
    
    lightMask = cv::Mat::zeros(mask.size(), CV_8UC3);
    cv::drawContours(lightMask,contours,-1,cv::Scalar(0, 255, 0),1);
//绿色绘制全部轮廓，厚度为1

for (const auto& contour : contours) 
{
    cv::RotatedRect rect = cv::minAreaRect(contour);
//获取最小外接矩形

    float length = std::max(rect.size.width, rect.size.height);
    float width  = std::min(rect.size.width, rect.size.height);
//获取矩形的长宽

if (width <= 0.0f) 
{
    continue;
}

float ratio = length / width;

if (ratio < 1.5f|| ratio > 3.0f) 
{
    continue;
}
// 初步排除不够细长的区域

double contourArea = cv::contourArea(contour);
double rectArea = length * width;
double fillRatio = contourArea / rectArea;
//计算轮廓面积与矩形面积的比值
if (fillRatio < 0.5) 
{
    continue;
}
// 排除轮廓面积与矩形面积比值过小的区域
LightBar light;

light.center = rect.center;
light.length = length;
light.width  = width;
light.rect   = rect;
// 将识别到的灯条信息存储到 lights 向量中
lights.push_back(light);
    cv::Point2f vertices[4];
//获取矩形的四个顶点

    rect.points(vertices);
//绘制矩形的四条边

    for (int i = 0; i < 4; i++)
    {
        cv::line(lightMask,vertices[i],vertices[(i + 1) % 4],cv::Scalar(0, 0, 255),1);
//红色绘制矩形边，厚度为1
    }
}

}
