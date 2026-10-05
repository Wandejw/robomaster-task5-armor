#pragma once

#include "DetectionTypes.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

// Values are stored as integers so OpenCV trackbars can edit them directly.
struct PreviewParams {
    int hMin = 80, hMax = 130, sMin = 30, sMax = 255, vMin = 150, vMax = 255;
    int kernelWidth = 1, kernelHeight = 5;
    int minAspect = 133, maxAspect = 1001;
    int minLength = 7;
    int minFill = 41, strongFill = 41;
    int previousLength = 70, previousDistance = 50;
    int maxTilt = 45;
    int pairLength = 50, pairDirection = 90;
    int maxAlong = 90, minSide = 165, maxSide = 460;
    int alongWeight = 1, targetSide = 249, directionWeight = 5, lengthWeight = 2;
};

inline void preview_preprocess(const cv::Mat& image, cv::Mat& mask,
                               const PreviewParams& p)
{
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(p.hMin, p.sMin, p.vMin),
                cv::Scalar(p.hMax, p.sMax, p.vMax), mask);
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT, cv::Size(std::max(1, p.kernelWidth),
                                 std::max(1, p.kernelHeight)));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
}

inline cv::Point2f preview_long_direction(const cv::RotatedRect& rect)
{
    cv::Point2f vertices[4];
    rect.points(vertices);
    cv::Point2f a = vertices[1] - vertices[0];
    cv::Point2f b = vertices[2] - vertices[1];
    cv::Point2f edge = a.dot(a) >= b.dot(b) ? a : b;
    float length = std::sqrt(edge.dot(edge));
    return length > 0.0f ? edge * (1.0f / length) : cv::Point2f();
}

inline void preview_detect(const cv::Mat& mask, cv::Mat& lightMask,
                           const std::vector<LightBar>& previousLights,
                           std::vector<LightBar>& lights, const PreviewParams& p)
{
    lights.clear();
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    lightMask = cv::Mat::zeros(mask.size(), CV_8UC3);
    cv::drawContours(lightMask, contours, -1, cv::Scalar(0, 255, 0), 1);
    for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float length = std::max(rect.size.width, rect.size.height);
        float width = std::min(rect.size.width, rect.size.height);
        if (width <= 0.0f || length < p.minLength) continue;
        float aspect = length / width;
        if (aspect < p.minAspect / 100.0f || aspect > p.maxAspect / 100.0f)
            continue;
        float fill = static_cast<float>(cv::contourArea(contour) / (length * width));
        if (fill < p.minFill / 100.0f) continue;

        if (fill < p.strongFill / 100.0f) {
            bool matched = false;
            for (const LightBar& previous : previousLights) {
                if (previous.fillRatio < p.strongFill / 100.0f) continue;
                float lengthRatio = std::min(length, previous.length) /
                                    std::max(length, previous.length);
                cv::Point2f delta = rect.center - previous.center;
                float maxDistance = p.previousDistance / 100.0f * length;
                if (lengthRatio >= p.previousLength / 100.0f &&
                    delta.dot(delta) <= maxDistance * maxDistance) {
                    matched = true;
                    break;
                }
            }
            if (!matched) continue;
        }

        cv::Point2f direction = preview_long_direction(rect);
        // Angle from vertical, using the absolute direction because edge order is arbitrary.
        float tilt = std::atan2(std::abs(direction.x), std::abs(direction.y)) *
                     180.0f / static_cast<float>(CV_PI);
        if (tilt > p.maxTilt) continue;

        LightBar light;
        light.center = rect.center;
        light.length = length;
        light.width = width;
        light.fillRatio = fill;
        light.rect = rect;
        lights.push_back(light);
        cv::Point2f vertices[4];
        rect.points(vertices);
        for (int i = 0; i < 4; ++i)
            cv::line(lightMask, vertices[i], vertices[(i + 1) % 4],
                     cv::Scalar(0, 0, 255), 1);
    }
}

inline void preview_match(const cv::Mat& lightMask, cv::Mat& lightMatch,
                          const std::vector<LightBar>& lights,
                          std::vector<cv::RotatedRect>& armorRects,
                          const PreviewParams& p)
{
    armorRects.clear();
    lightMatch = lightMask.clone();
    struct Candidate {
        std::size_t first, second;
        double loss;
        cv::RotatedRect rect;
    };
    std::vector<Candidate> candidates;
    for (std::size_t i = 0; i < lights.size(); ++i) {
        for (std::size_t j = i + 1; j < lights.size(); ++j) {
            float lengthRatio = std::min(lights[i].length, lights[j].length) /
                                std::max(lights[i].length, lights[j].length);
            if (lengthRatio < p.pairLength / 100.0f) continue;
            float averageLength = (lights[i].length + lights[j].length) / 2.0f;
            cv::Point2f a = preview_long_direction(lights[i].rect);
            cv::Point2f b = preview_long_direction(lights[j].rect);
            if (a.dot(a) == 0.0f || b.dot(b) == 0.0f) continue;
            float similarity = std::abs(a.dot(b));
            if (similarity < p.pairDirection / 100.0f) continue;
            cv::Point2f delta = lights[j].center - lights[i].center;
            float along = std::abs(delta.dot(a)) / averageLength;
            float side = std::abs(a.x * delta.y - a.y * delta.x) / averageLength;
            if (along > p.maxAlong / 100.0f ||
                side < p.minSide / 100.0f || side > p.maxSide / 100.0f)
                continue;
            double loss = along * p.alongWeight +
                          std::abs(side - p.targetSide / 100.0f) +
                          std::abs(similarity - 1.0f) * p.directionWeight +
                          std::abs(lengthRatio - 1.0f) * p.lengthWeight;
            cv::Point2f first[4], second[4];
            lights[i].rect.points(first);
            lights[j].rect.points(second);
            std::vector<cv::Point2f> points;
            for (int k = 0; k < 4; ++k) {
                points.push_back(first[k]);
                points.push_back(second[k]);
            }
            candidates.push_back({i, j, loss, cv::minAreaRect(points)});
        }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) { return a.loss < b.loss; });
    std::vector<bool> used(lights.size(), false);
    for (const Candidate& candidate : candidates) {
        if (used[candidate.first] || used[candidate.second]) continue;
        used[candidate.first] = used[candidate.second] = true;
        armorRects.push_back(candidate.rect);
        cv::Point2f vertices[4];
        candidate.rect.points(vertices);
        for (int i = 0; i < 4; ++i)
            cv::line(lightMatch, vertices[i], vertices[(i + 1) % 4],
                     cv::Scalar(0, 255, 255), 2);
    }
}
