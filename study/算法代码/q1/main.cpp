#include <opencv2/opencv.hpp>
#include "function.h"
int main()
{
    cv::Mat image;
    getImage(image);
    //得到一个图像
    //输入：图像
    //输出三个参数
    swapChannel_repeat(image);
    //交换代码
    //固定数学格式，输入三个数字、输出三个数字
    outputImage(image);
    //将三个数字转化回图像
}
