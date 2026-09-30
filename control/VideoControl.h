#pragma once
#include <string>
#include <opencv2/videoio.hpp>

void init_progress_bar(const std::string& windowName, int totalFrames);

void update_progress_bar(const cv::VideoCapture& capture,
                         const std::string& windowName);
bool seek_progress_bar(cv::VideoCapture& capture,
                        const std::string& windowName);
