#include "main.h"

int main()
{
    cv::Mat image;
    getImage(image);
    //得到一个图像
    //输入：图像
    repeat_invertColor(image);
     outputImage(image);
    //将三个数字转化回图像
    return 0;
}