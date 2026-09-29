#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include "../common/DetectionTypes.h"

void armor_match(const cv::Mat& lightMask, cv::Mat& lightMatch,const std::vector<LightBar>& lights,std::vector<cv::RotatedRect>& armorRects);
