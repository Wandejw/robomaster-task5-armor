#pragma once
#include <opencv2/core.hpp>

struct LightBar
{
    cv::Point2f center;       // 中心坐标，包含 x、y
    float length;            // 长边
    float width;             // 短边
    float fillRatio = 0.0f;  // 当前帧轮廓的填充率
    cv::RotatedRect rect;     // 最小外接旋转矩形
};
