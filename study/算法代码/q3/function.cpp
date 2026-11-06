#include "main.h"
void binariZation(unsigned char* pixelData)
{
    double Y = 0.2126*pixelData[2] + 0.7152*pixelData[1] + 0.0722*pixelData[0];
    unsigned char gray = static_cast<unsigned char>(std::round(Y));
    //转化类型
    if (gray<128)
    {
        pixelData[2] =0;
        pixelData[1] =0;
        pixelData[0] =0;
    }
    else
    {
        pixelData[2] =255;
        pixelData[1] =255;
        pixelData[0] =255;
    }
}

void binariZation_repeat(cv::Mat& image)
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
    binariZation(pixelData);
    //某一个功能（本题灰度化）
    }
}

