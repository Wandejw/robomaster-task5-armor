#pragma once

#include <opencv2/core/mat.hpp>

void getImage(cv::Mat& image);
void swapChannel_repeat(cv::Mat& image);
void outputImage(cv::Mat image);
