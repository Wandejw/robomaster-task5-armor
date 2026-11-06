#pragma once
#include <opencv2/core.hpp>

struct LightBar
{
    cv::Point2f center;       // 中心坐标，包含 x、y
    float length;            // 长边
    float width;             // 短边
    float fillRatio = 0.0f;  // 当前帧轮廓的填充率
    bool redOnly = false;    // 圆形蓝点只显示红框，不参与灯条配对
    bool isRed = false;       // 灯条所属的目标颜色
    cv::RotatedRect rect;     // 最小外接旋转矩形
};
