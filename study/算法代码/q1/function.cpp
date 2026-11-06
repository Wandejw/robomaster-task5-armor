#include <opencv2/opencv.hpp>
#include "function.h"

void getImage(cv::Mat& image)
{
    image = cv::imread("input.jpg");
}

void swapChannel(unsigned char* pixelData)
{
    unsigned char temp=0;
    temp=pixelData[0];
    pixelData[0]=pixelData[2];
    pixelData[2]=temp;
    //交换代码局部：
}

void swapChannel_repeat(cv::Mat& image)
{
    //轮询
   for(int i=0;i<image.cols*image.rows;i++)
    {
    int row=i/image.cols;
    int col=i%image.cols;
    unsigned char* rowData;
    rowData=image.data+row*image.step[0];
    unsigned char* pixelData;
    pixelData=rowData+col*3;
    swapChannel(pixelData);
    }
}

void outputImage(cv::Mat image)
{
    cv::imshow("result",image);
    cv::waitKey(0);
}


