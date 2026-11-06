#include <opencv2/opencv.hpp>
#include "main.h"

void getImage(cv::Mat& image)
{
    image = cv::imread("input.jpg");
}

void outputImage(cv::Mat image)
{
    cv::imshow("result",image);
    cv::waitKey(0);
}



