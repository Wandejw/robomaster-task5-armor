#include "main.h"
#include <algorithm>

void analyzeImage(cv::Mat& image)
{
cv::Mat result = image.clone();
    for(int row=0;row<image.rows;row++)
    {
       for(int col=0;col<image.cols;col++)
       {
         int valuesB[9] = {};
         int valuesG[9] = {};
         int valuesR[9] = {};
         int index=0;
         
         for(int drow=-1;drow<2;drow++)
         {
           for(int dcol=-1;dcol<2;dcol++)
           {   
            int nearrow=row+drow;
            int nearcol=col+dcol;
            int nearB=0;
            int nearG=0;
            int nearR=0;
              if( (nearrow<0||nearrow>=image.rows)&&(nearcol<0||nearcol>=image.cols))
              {
                // Zero Padding: keep the three channel values at 0.
              }
              else if (nearcol<0||nearcol>=image.cols||nearrow<0||nearrow>=image.rows)
              {
                // Zero Padding: keep the three channel values at 0.
              }
              else
              {
                cv::Vec3b& pixelData =image.at<cv::Vec3b>(nearrow, nearcol);
                nearB=pixelData[0];
                nearG=pixelData[1];
                nearR=pixelData[2];
              }
              valuesB[index]=nearB;
              valuesG[index]=nearG;
              valuesR[index]=nearR;
              index++;
            }
         }
         std::sort(valuesB, valuesB+9);
         std::sort(valuesG, valuesG+9);
         std::sort(valuesR, valuesR+9);
         result.at<cv::Vec3b>(row, col)[0] = cv::saturate_cast<uchar>(valuesB[4]);
         result.at<cv::Vec3b>(row, col)[1] = cv::saturate_cast<uchar>(valuesG[4]);
         result.at<cv::Vec3b>(row, col)[2] = cv::saturate_cast<uchar>(valuesR[4]);
        }
    }
    result.copyTo(image);
}


