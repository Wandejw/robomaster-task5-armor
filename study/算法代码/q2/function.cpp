#include "main.h"
void Grayscale(unsigned char* pixelData)
{
    double Y = 0.2126*pixelData[2] + 0.7152*pixelData[1] + 0.0722*pixelData[0];
    unsigned char gray = static_cast<unsigned char>(std::round(Y));
    //转化类型
    pixelData[2] =gray;
    pixelData[1] =gray;
    pixelData[0] =gray;
}

void Grayscale_repeat(cv::Mat& image)
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
    Grayscale(pixelData);
    //某一个功能（本题灰度化）
    }
}

