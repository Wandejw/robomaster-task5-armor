#include <opencv2/core.hpp>      // cv::Mat
#include <opencv2/videoio.hpp>   // cv::VideoCapture
#include <opencv2/highgui.hpp>   // imshow、waitKey、destroyAllWindows
#include <iostream>             // std::cerr
#include <string>               // std::string
#include "ImageProcessor.h"
#include "LightDetector.h"
#include "ArmorMatcher.h"
#include "InputHandle.h"
#include "VideoControl.h"

int main()
{
//输入视频，输出照片
bool paused = false;  //   false：播放；true：暂停
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
int totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
init_progress_bar("Result", totalFrames);

cv::Mat image;
cv::Mat mask;
cv::Mat lightMask;
std::vector<LightBar> lights;
std::vector<cv::RotatedRect> armorRects;
cv::Mat lightMatch;
cv::Mat handleImage;
cv::VideoWriter writer;
while (true)
{
    if (!paused)
    {
        if (!capture.read(image))
        {
            break; 
        }
        preprocess_image(image,mask);

//灯光识别
        light_detector(mask,lightMask,lights);

//灯光匹配
        armor_match(lightMask,lightMatch,lights,armorRects);

//绘制显示,
        draw_display(image, armorRects, handleImage);

//显示图像到result窗口
    }

    // 暂停时也继续显示上一帧、接收按键。
    cv::imshow("Result", handleImage);
    int key = cv::waitKey(30);

    if (key == 27)
    {
        break;  // Esc 退出
    }

    if (key == ' ')
    {
        paused = !paused;  // 空格切换暂停与播放
    }
}

//处理退出
capture.release(); 
cv::destroyAllWindows();
    return 0;
}
}
