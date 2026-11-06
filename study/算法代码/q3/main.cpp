#include "main.h"

int main()
{
    cv::Mat image;
    getImage(image);
    //得到一个图像
    //输入：图像
    binariZation_repeat(image);
    //输入：图像
    //使图像会度化
     outputImage(image);
    //将三个数字转化回图像
    return 0;
}