#include "VideoControl.h"
#include <opencv2/highgui.hpp>

void init_progress_bar(const std::string& windowName, int totalFrames)
{
    if (totalFrames <= 1)
    {
        return;
    }

    cv::namedWindow(windowName);
    cv::createTrackbar("Frame", windowName, nullptr, totalFrames - 1);
}
