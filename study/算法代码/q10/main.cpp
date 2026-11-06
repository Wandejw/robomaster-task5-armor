#include "main.h"

int main()
{
    cv::Mat image;
    getImage(image);
    analyzeImage(image);
    outputImage(image);
    //将三个数字转化回图像
    return 0;
}