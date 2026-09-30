#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include "../common/DetectionTypes.h"
#include <cmath>

void light_detector(const cv::Mat& mask, cv::Mat& lightMask,std::vector<LightBar>& lights);