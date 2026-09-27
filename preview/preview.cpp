#include "ImageProcessor.h"
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string>
#include "LightDetector.h"
#include "ArmorMatcher.h"
int main(int argc, char** argv)
{
    const bool checkOnly = argc > 1 && std::string(argv[1]) == "--check";
    const std::string videoPath =
        "/home/wandejw/桌面/rp作业/task5/drone_blue3.blue-only.avi";
    cv::VideoCapture capture(videoPath);
    if (!capture.isOpened()) {
        std::cerr << "无法打开视频：" << videoPath << '\n';
        return 1;
    }

    cv::Mat image;
    cv::Mat mask;
    cv::Mat lightMask;
    cv::Mat lightMatch;
    cv::Mat yellowMask;
    cv::Mat yellowOnly;
    std::vector<LightBar> lights;
    int frameCount = 0;
    while (capture.read(image)) {
        preprocess_image(image, mask);
        if (mask.empty()) {
            std::cerr << "preprocess_image 没有生成 mask，请保存并检查 ImageProcessor.cpp。\n";
            return 2;
        }
        if (mask.type() != CV_8UC1 || mask.size() != image.size()) {
            std::cerr << "mask 应为与原图同尺寸的 CV_8UC1 图像。\n";
            return 3;
        }
        light_detector(mask, lightMask, lights);
        if (lightMask.empty() || lightMask.type() != CV_8UC3 ||
            lightMask.size() != image.size()) {
            std::cerr << "light_detector 应生成与原图同尺寸的 CV_8UC3 轮廓图。\n";
            return 5;
        }
        armor_match(lightMask, lightMatch, lights);
        if (lightMatch.empty() || lightMatch.type() != CV_8UC3 ||
            lightMatch.size() != image.size()) {
            std::cerr << "armor_match 应生成与原图同尺寸的 CV_8UC3 配对图。\n";
            return 6;
        }
        // 只保留配对图中黄色连线，每帧使用新的黑底避免残留。
        cv::inRange(lightMatch, cv::Scalar(0, 255, 255),
                    cv::Scalar(0, 255, 255), yellowMask);
        yellowOnly = cv::Mat::zeros(lightMatch.size(), lightMatch.type());
        lightMatch.copyTo(yellowOnly, yellowMask);
        ++frameCount;
        if (checkOnly) {
            std::cout << "读取及预处理成功：" << image.cols << 'x'
                      << image.rows << "，mask 非零像素数："
                      << cv::countNonZero(mask)
                      << "，候选灯条数：" << lights.size()
                      << "，黄色连线像素数：" << cv::countNonZero(yellowMask)
                      << "，合并图及纯黄色图已生成。\n";
            return 0;
        }

        cv::imshow("Original", image);
        cv::imshow("Mask", mask);
        cv::imshow("Matches", lightMatch);
        cv::imshow("YellowOnly", yellowOnly);
        if (cv::waitKey(30) == 27) {
            break;
        }
    }
    cv::destroyAllWindows();
    if (frameCount == 0) {
        std::cerr << "视频打开成功，但没有读到图像帧。\n";
        return 4;
    }
    std::cout << "已预览 " << frameCount << " 帧。\n";
    return 0;
}
