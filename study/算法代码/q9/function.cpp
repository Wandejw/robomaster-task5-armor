#include "main.h"
#include <algorithm>

void analyzeImage(cv::Mat& image)
{
cv::Mat result = image.clone();
double sigma = 1.3;
double kernel[3][3] = {};
double weightSum = 0.0;

for (int drow = -1; drow <2; drow++) 
{
    for (int dcol = -1; dcol <2; dcol++) 
    {
        double weight = std::exp(-(drow * drow + dcol * dcol) / (2.0 * sigma * sigma));
        kernel[drow + 1][dcol + 1] = weight;
        weightSum += weight;
    }
}

for (int kr = 0; kr < 3; kr++) 
{
    for (int kc = 0; kc < 3; kc++) 
    {
        kernel[kr][kc] /= weightSum;
    }
}    
    for(int row=0;row<image.rows;row++)
    {
       for(int col=0;col<image.cols;col++)
       {
         double sumB=0;
         double sumG=0;
         double sumR=0;
         
         for(int drow=-1;drow<2;drow++)
         {
           for(int dcol=-1;dcol<2;dcol++)
           {   
            int nearrow=row+drow;
            int nearcol=col+dcol;
              if( (nearrow<0||nearrow>=image.rows)&&(nearcol<0||nearcol>=image.cols))
              {
              }
              else if (nearcol<0||nearcol>=image.cols||nearrow<0||nearrow>=image.rows)
              {
              }
              else
              {
                cv::Vec3b& pixelData =image.at<cv::Vec3b>(nearrow, nearcol);
                int nearB=pixelData[0];
                int nearG=pixelData[1];
                int nearR=pixelData[2];
               sumB+=nearB*kernel[drow+1][dcol+1];
               sumG+=nearG*kernel[drow+1][dcol+1];
               sumR+=nearR*kernel[drow+1][dcol+1];
              }
            }
         }
         result.at<cv::Vec3b>(row, col)[0] = cv::saturate_cast<uchar>(sumB);
         result.at<cv::Vec3b>(row, col)[1] = cv::saturate_cast<uchar>(sumG);
         result.at<cv::Vec3b>(row, col)[2] = cv::saturate_cast<uchar>(sumR);
        }
    }
    result.copyTo(image);
}



