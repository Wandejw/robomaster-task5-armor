#include "PreviewPipeline.h"
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
int main(int argc, char** argv)
{
    const bool checkOnly = argc > 1 && std::string(argv[1]) == "--check";
    const std::string videoPath =
        "/home/wandejw/桌面/rp作业/task5/drone_blue3.avi";
    cv::VideoCapture capture(videoPath);
    if (!capture.isOpened()) {
        std::cerr << "无法打开视频：" << videoPath << '\n';
        return 1;
    }

    int totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    const bool hasProgressBar = totalFrames > 1;
    PreviewParams params;
    std::vector<int*> controls;
    if (!checkOnly) {
        cv::namedWindow("Original");
        cv::namedWindow("Matches");
        if (hasProgressBar) {
            cv::createTrackbar("Frame", "Original", nullptr, totalFrames - 1);
            cv::createTrackbar("Frame", "Matches", nullptr, totalFrames - 1);
        }
        cv::namedWindow("Parameters", cv::WINDOW_NORMAL);
        cv::resizeWindow("Parameters", 800, 800);
        cv::imshow("Parameters", cv::Mat::zeros(80, 800, CV_8UC3));
        auto slider = [&](const char* label, int& value, int maximum) {
            controls.push_back(&value);
            cv::createTrackbar(label, "Parameters", &value, maximum);
        };
        slider("H下限：颜色起点", params.hMin, 179);
        slider("H上限：颜色终点", params.hMax, 179);
        slider("S下限：最低饱和度", params.sMin, 255);
        slider("S上限：最高饱和度", params.sMax, 255);
        slider("V下限：最低亮度", params.vMin, 255);
        slider("V上限：最高亮度", params.vMax, 255);
        slider("核宽：横向连接范围", params.kernelWidth, 21);
        slider("核高：竖向连接范围", params.kernelHeight, 21);
        slider("灯条最短长度：像素乘10", params.minLength, 400);
        slider("灯条长宽比下限：数值除以100", params.minAspect, 1000);
        slider("灯条长宽比上限：数值除以100", params.maxAspect, 1500);
        slider("填充率下限：轮廓占矩形百分比", params.minFill, 100);
        slider("前帧黄框尺寸比：最低百分比", params.previousLength, 100);
        slider("前帧黄框中心距：当前框长度百分比", params.previousDistance, 200);
        slider("配对延续奖励：数值除以100", params.continuityBonus, 500);
        slider("最大倾角：偏离竖直的角度", params.maxTilt, 90);
        slider("配对长度比：两灯条最小百分比", params.pairLength, 100);
        slider("配对方向相似度：最低百分比", params.pairDirection, 100);
        slider("最大纵向错位：灯条长度百分比", params.maxAlong, 300);
        slider("最小横向间距：灯条长度百分比", params.minSide, 500);
        slider("最大横向间距：灯条长度百分比", params.maxSide, 500);
        slider("纵向错位权重：越大越优先对齐", params.alongWeight, 30);
        slider("理想横向间距：灯条长度百分比", params.targetSide, 500);
        slider("横向间距偏差权重", params.sideWeight, 30);
        slider("方向差权重：越大越优先平行", params.directionWeight, 30);
        slider("长度差权重：越大越优先等长", params.lengthWeight, 30);
    }

    cv::Mat image;
    cv::Mat mask;
    cv::Mat lightMask;
    cv::Mat lightMatch;
    cv::Mat lightBoxes;
    cv::Mat resultBoxes;
    std::vector<LightBar> lights;
    std::vector<cv::RotatedRect> armorRects;
    int frameCount = 0;
    bool paused = false;
    int shownFrame = -1;
    int stepFrame = -1;
    std::vector<cv::RotatedRect> previousArmorForCurrentFrame;
    std::vector<cv::RotatedRect> olderArmorForCurrentFrame;
    std::vector<int> appliedControls;
    while (true) {
        bool seekRequested = false;
        int requestedFrame = -1;
        if (!checkOnly && hasProgressBar && shownFrame >= 0) {
            int originalFrame = cv::getTrackbarPos("Frame", "Original");
            int matchesFrame = cv::getTrackbarPos("Frame", "Matches");
            requestedFrame = stepFrame;
            stepFrame = -1;
            if (requestedFrame < 0 && originalFrame != shownFrame) requestedFrame = originalFrame;
            else if (requestedFrame < 0 && matchesFrame != shownFrame) requestedFrame = matchesFrame;
            if (requestedFrame >= 0) {
                seekRequested = capture.set(cv::CAP_PROP_POS_FRAMES,
                                            std::max(0, requestedFrame - 8));
            }
        }
        std::vector<int> currentControls;
        for (int* value : controls) currentControls.push_back(*value);
        bool parametersChanged = currentControls != appliedControls;
        bool newFrame = !paused || seekRequested;
        if (newFrame || parametersChanged) {
            if (seekRequested) {
                lights.clear();
                armorRects.clear();
                previousArmorForCurrentFrame.clear();
                olderArmorForCurrentFrame.clear();
                // 跳帧后先处理目标帧前的八帧，让配对选择稳定到与顺序播放一致。
                for (int frame = std::max(0, requestedFrame - 8);
                     frame < requestedFrame; ++frame) {
                    cv::Mat warmupImage, warmupMask, warmupLightMask, warmupMatch;
                    std::vector<LightBar> warmupLights;
                    std::vector<cv::RotatedRect> warmupRects;
                    if (!capture.read(warmupImage)) break;
                    preview_preprocess(warmupImage, warmupMask, params);
                    preview_detect_dual(warmupMask, warmupLightMask, warmupLights, params);
                    preview_match(warmupLightMask, warmupMatch, warmupLights,
                                  warmupRects, params, armorRects,
                                  previousArmorForCurrentFrame);
                    previousArmorForCurrentFrame = armorRects;
                    armorRects = warmupRects;
                }
            }
            if (newFrame) {
                if (!capture.read(image)) break;
                olderArmorForCurrentFrame = previousArmorForCurrentFrame;
                previousArmorForCurrentFrame = armorRects;
            }
            if (image.empty()) continue;
            preview_preprocess(image, mask, params);
            if (mask.empty()) {
                std::cerr << "preprocess_image 没有生成 mask，请保存并检查 ImageProcessor.cpp。\n";
                return 2;
            }
            if (mask.type() != CV_8UC1 || mask.size() != image.size()) {
                std::cerr << "mask 应为与原图同尺寸的 CV_8UC1 图像。\n";
                return 3;
            }
            preview_detect_dual(mask, lightMask, lights, params);
            if (lightMask.empty() || lightMask.type() != CV_8UC3 ||
                lightMask.size() != image.size()) {
                std::cerr << "light_detector 应生成与原图同尺寸的 CV_8UC3 轮廓图。\n";
                return 5;
            }
            preview_match(lightMask, lightMatch, lights, armorRects, params,
                          previousArmorForCurrentFrame, olderArmorForCurrentFrame);
            if (lightMatch.empty() || lightMatch.type() != CV_8UC3 ||
                lightMatch.size() != image.size()) {
                std::cerr << "armor_match 应生成与原图同尺寸的 CV_8UC3 配对图。\n";
                return 6;
            }
            // 在当前原始视频帧上画框，避免黑底调试图遮住真实目标。
            lightBoxes = image.clone();
            for (const LightBar& light : lights) {
                cv::Point2f vertices[4];
                light.rect.points(vertices);
                for (int i = 0; i < 4; ++i) {
                    cv::line(lightBoxes, vertices[i], vertices[(i + 1) % 4],
                             cv::Scalar(0, 0, 255), 2);
                }
            }
            resultBoxes = lightBoxes.clone();
            for (const cv::RotatedRect& armorRect : armorRects) {
                cv::Point2f vertices[4];
                armorRect.points(vertices);
                for (int i = 0; i < 4; ++i) {
                    cv::line(resultBoxes, vertices[i], vertices[(i + 1) % 4],
                             cv::Scalar(0, 255, 255), 2);
                }
            }
            if (!checkOnly && newFrame && hasProgressBar) {
                shownFrame = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES)) - 1;
                cv::setTrackbarPos("Frame", "Original", shownFrame);
                cv::setTrackbarPos("Frame", "Matches", shownFrame);
            }
            if (newFrame) ++frameCount;
            appliedControls = currentControls;
            if (checkOnly) {
                std::cout << "读取及预处理成功：" << image.cols << 'x'
                          << image.rows << "，mask 非零像素数："
                          << cv::countNonZero(mask)
                          << "，候选灯条数：" << lights.size()
                          << "，装甲板框数：" << armorRects.size()
                          << "，原图红框及原图红黄框已生成。\n";
                return 0;
            }

        }

        cv::imshow("Original", image);
        cv::imshow("LightBoxes", lightBoxes);
        cv::imshow("Matches", lightMatch);
        cv::imshow("ResultBoxes", resultBoxes);
        // 暂停期间仍接收按键；四个窗口共用同一暂停状态。
        int key = cv::waitKeyEx(30);
        if (key == 27) {
            break;
        }
        if (key == ' ' || key == 'p' || key == 'P') {
            paused = !paused;
            std::cout << (paused ? "已暂停（空格继续）" : "继续播放") << std::endl;
        }
        if (hasProgressBar && shownFrame >= 0 &&
            (key == 65361 || key == 16777234 || key == 2424832 ||
             key == 65363 || key == 16777236 || key == 2555904)) {
            const int direction = (key == 65361 || key == 16777234 || key == 2424832) ? -1 : 1;
            stepFrame = std::max(0, std::min(totalFrames - 1, shownFrame + direction));
            paused = true;
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
