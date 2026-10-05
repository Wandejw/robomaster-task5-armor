#include "LightDetector.h"
#include <opencv2/imgproc.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

void light_detector(const cv::Mat& mask, cv::Mat& lightMask,std::vector<LightBar>& lights)
// 识别灯条轮廓并绘制到 lightMask 上，同时将识别到的灯条信息存储到 lights 向量中
{
    // 先保存上一帧的灯条信息，以便在需要时进行比较或跟踪
    std::vector<LightBar> previousLights = lights;
    // 先清空 lights 向量，以便存储当前帧识别到的灯条信息
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

if (ratio < 1.5f|| ratio > 9.01f) 
{
    continue;
}
// 初步排除不够细长的区域

double contourArea = cv::contourArea(contour);
double rectArea = length * width;
double fillRatio = contourArea / rectArea;
//计算轮廓面积与矩形面积的比值
if (fillRatio < 0.45) 
{
    continue;
}
// 排除轮廓面积与矩形面积比值过小的区域
// 通过以上条件筛选出的轮廓被认为是灯条，接下来将其信息存储到 lights 向量中，并绘制到 lightMask 上
LightBar light;

light.center = rect.center;
light.length = length;
light.width  = width;
light.rect   = rect;
light.fillRatio = static_cast<float>(fillRatio);

cv::Point2f vertices[4];
rect.points(vertices);
//绘制矩形的四条边

cv::Point2f edgeA = vertices[1] - vertices[0];
cv::Point2f edgeB = vertices[2] - vertices[1];
float lengthA2 = edgeA.dot(edgeA);
float lengthB2 = edgeB.dot(edgeB);
//计算两条边的长度平方
cv::Point2f longEdge = (lengthA2 >= lengthB2) ? edgeA : edgeB;
//选择较长的边作为长边
// 相对竖直方向偏移超过 25°，跳过这个候选灯条

// 计算灯条的填充率
if (fillRatio < 0.45) 
{
    bool matchedPreviousStrong = false;
// 检查上一帧中是否有强候选灯条与当前灯条匹配
    for (const LightBar& previous : previousLights) 
    {
        // 如果上一帧的灯条填充率小于 0.45，则认为它是弱候选，不能继续担保下一帧
        if (previous.fillRatio < 0.45f)
         {
            continue;  
        }
// 计算当前灯条与上一帧灯条的长度比和中心点距离
        float lengthRatio =
            std::min(length, previous.length) /
            std::max(length, previous.length);
// 计算当前灯条与上一帧灯条的中心点距离
        cv::Point2f delta = rect.center - previous.center;
        float maxDistance = 0.5f * length;
// 如果长度比大于等于 0.7 且中心点距离小于等于最大距离，则认为匹配成功
        if (lengthRatio >= 0.7f &&
            delta.dot(delta) <= maxDistance * maxDistance)
            {
            matchedPreviousStrong = true;
            break;
            }
    }

    if (!matchedPreviousStrong) 
    {
        continue;
    }
}
float tiltDegrees = std::atan2(std::abs(longEdge.x), std::abs(longEdge.y))
                    * 180.0f / static_cast<float>(CV_PI);
if (tiltDegrees > 25.0f)
{ 
    continue;
}

    rect.points(vertices);

// 将识别到的灯条信息存储到 lights 向量中
lights.push_back(light);
   


    for (int i = 0; i < 4; i++)
    {
        cv::line(lightMask,vertices[i],vertices[(i + 1) % 4],cv::Scalar(0, 0, 255),1);
//红色绘制矩形边，厚度为1
    }
}

}
