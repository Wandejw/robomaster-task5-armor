#include "VideoControl.h"
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

void init_progress_bar(const std::string& windowName, int totalFrames)//定义一个函数，用于初始化进度条
// windowName 窗口名称，totalFrames 总帧数

// 如果总帧数小于等于 1，则不会创建窗口和滑动条
{
    if (totalFrames <= 1)
    {
        return;
    }

    // 创建一个窗口，窗口名称为 windowName
    cv::namedWindow(windowName);
    // 创建一个滑动条，滑动条名称为 "Frame"，范围为 0 到 totalFrames - 1
    // 回调函数为 nullptr表示不需要回调函数
    cv::createTrackbar("Frame", windowName, nullptr, totalFrames - 1);
}

void update_progress_bar(const cv::VideoCapture& capture,
                         const std::string& windowName)
{
    // 初始化函数没有创建滑条时，不尝试更新它。
    if (capture.get(cv::CAP_PROP_FRAME_COUNT) <= 1)
    {
        return;
    }
    // 获取当前帧索引，减去 1 是因为帧索引从 0 开始
    int currentFrame = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES)) - 1;
    // 设置滑动条的位置为当前帧索引
    // cv::setTrackbarPos 函数用于设置滑动条的位置
    // windowName 窗口名称，"Frame" 滑动条名称，currentFrame 当前帧索引
    cv::setTrackbarPos("Frame", windowName, currentFrame);
}

// 该函数用于在视频中跳转到指定帧
// capture 视频捕获对象，windowName 窗口名称
bool seek_progress_bar(cv::VideoCapture& capture,const std::string& windowName)
{
    // 如果视频总帧数小于等于 1，则不会进行跳转
    if (capture.get(cv::CAP_PROP_FRAME_COUNT) <= 1)
    {
        return false;
    }

// 获取滑动条的位置，即用户希望跳转到的帧索引
    int sliderFrame = cv::getTrackbarPos("Frame", windowName);
// 获取当前帧索引，减去 1 是因为帧索引从 0 开始
    int currentFrame =static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES)) - 1;

// 如果滑动条位置与当前帧索引不一致，则尝试跳转到滑动条位置
    if (sliderFrame != currentFrame)
    {
        return capture.set(cv::CAP_PROP_POS_FRAMES, sliderFrame);
    }

    return false;
}
