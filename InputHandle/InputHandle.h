#pragma once
#include <opencv2/imgproc.hpp>  // cv::line
#include <opencv2/core.hpp>
#include <vector>
#include <opencv2/videoio.hpp>
#include <string>
void draw_display(const cv::Mat& lightMatch, const std::vector<cv::RotatedRect>& armorRects,cv::Mat& handleImage);


// 第一次调用创建输出视频，后续调用逐帧写入；返回是否成功。
bool save_video(cv::VideoWriter& writer, const cv::Mat& image, double fps,
                const std::string& outputPath = "build/armor_result.avi");
