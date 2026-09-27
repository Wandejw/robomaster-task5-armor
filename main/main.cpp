#include <opencv2/core.hpp>      // cv::Mat
#include <opencv2/videoio.hpp>   // cv::VideoCapture
#include <opencv2/highgui.hpp>   // imshow、waitKey、destroyAllWindows
#include <iostream>             // std::cerr
#include <string>               // std::string
#include "ImageProcessor.h"
#include "LightDetector.h"
#include "ArmorMatcher.h"
#include "InputHandle.h"

int main()
{
//输入视频，输出照片
const std::string videoPath ="/home/wandejw/桌面/rp作业/task5/drone_blue3.avi";
cv::VideoCapture capture(videoPath);
bool success=capture.isOpened();
if (!success) 
{
    std::cerr << "无法打开视频：" << videoPath << '\n';
    return 1;
}
else
{
cv::Mat image;
cv::Mat mask;
cv::Mat lightMask;
cv::Mat lightMatch;
cv::Mat handleImage;

while (capture.read(image))
{   
//照片处理
preprocess_image(image,mask);

//灯光识别
light_detector(mask,lightMask);

//灯光匹配
armor_match(lightMask,lightMatch);

//绘制显示
draw_display(lightMatch,handleImage);

//显示图像到result窗口
cv::imshow("Result",handleImage);

if (cv::waitKey(30) == 27)
{
    break;
}
}

//处理退出
capture.release(); 
cv::destroyAllWindows();
    return 0;
}
}
