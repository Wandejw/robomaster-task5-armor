#include "main.h"
void averagrPooling(cv::Mat& image,int blockRow,int blockCol)
{
    int sumB=0;
    int sumR=0;
    int sumG=0;
    for(int offsetrow=0;offsetrow<8;offsetrow++)
    {
        int rowNow = offsetrow + blockRow;
        for(int offsetcol=0;offsetcol<8;offsetcol++)
        {
            int colNow=offsetcol+blockCol;
            cv::Vec3b& pixelData =image.at<cv::Vec3b>(rowNow, colNow);
            sumB+=pixelData[0];
            sumG+=pixelData[1];
            sumR+=pixelData[2];        
        }
    }
    int averageB = sumB / 64;
    int averageG = sumG / 64;
    int averageR = sumR / 64;
    for (int offsetrow = 0; offsetrow < 8; offsetrow++)
    {
    int rowNow = blockRow + offsetrow;

    for (int offsetcol = 0; offsetcol < 8; offsetcol++)
    {
        int colNow = blockCol + offsetcol;
        cv::Vec3b& pixel =image.at<cv::Vec3b>(rowNow, colNow);
        pixel[0] = averageB;
        pixel[1] = averageG;
        pixel[2] = averageR;
    }
    }
}
    

void repeat_averagrPooling(cv::Mat& image)
{
    for (int blockRow = 0;blockRow < image.rows;blockRow += 8)
    {
        for (int blockCol = 0;blockCol < image.cols;blockCol += 8)
        {
            averagrPooling(image, blockRow, blockCol);
        }
    }
}

