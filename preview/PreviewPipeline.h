#pragma once

#include "DetectionTypes.h"
#include "PairContinuity.h"
#include "PairSelection.h"
#include "CompactLightCandidate.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <utility>

// Values are stored as integers so OpenCV trackbars can edit them directly.
struct PreviewParams {
    int hMin = 80, hMax = 130, sMin = 30, sMax = 255, vMin = 150, vMax = 255;
    int kernelWidth = 1, kernelHeight = 5;
    int minAspect = 133, maxAspect = 1001;
    int minLength = 40;
    int minFill = 33;
    int previousLength = 70, previousDistance = 50, continuityBonus = 500;
    int maxTilt = 45;
    int pairLength = 45, pairDirection = 88;
    int maxAlong = 170, minSide = 115, maxSide = 470;
    int alongWeight = 3, targetSide = 290, sideWeight = 3;
    int directionWeight = 5, lengthWeight = 2;
};

inline void preview_preprocess(const cv::Mat& image, cv::Mat& mask,
                               const PreviewParams& p)
{
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(p.hMin, p.sMin, p.vMin),
                cv::Scalar(p.hMax, p.sMax, p.vMax), mask);
    cv::Mat redLow, redHigh;
    cv::inRange(hsv, cv::Scalar(0, p.sMin, p.vMin),
                cv::Scalar(20, p.sMax, p.vMax), redLow);
    cv::inRange(hsv, cv::Scalar(170, p.sMin, p.vMin),
                cv::Scalar(179, p.sMax, p.vMax), redHigh);
    cv::Mat redMask;
    cv::bitwise_or(redLow, redHigh, redMask);
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT, cv::Size(std::max(1, p.kernelWidth),
                                 std::max(1, p.kernelHeight)));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(redMask, redMask, cv::MORPH_CLOSE, kernel);
    cv::Mat blueMask = mask.clone();
    mask.setTo(128, redMask);
    mask.setTo(255, blueMask);
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
                           std::vector<LightBar>& lights, const PreviewParams& p,
                           bool redMask = false)
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
        if (width <= 0.0f || length < p.minLength / 10.0f) continue;
        float aspect = length / width;
        if (aspect < p.minAspect / 100.0f || aspect > p.maxAspect / 100.0f)
            continue;
        float fill = static_cast<float>(cv::contourArea(contour) / (length * width));
        if (fill < p.minFill / 100.0f) continue;

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
    if (redMask) for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float length = std::max(rect.size.width, rect.size.height);
        float width = std::min(rect.size.width, rect.size.height);
        if (width <= 0.0f) continue;
        float fill = static_cast<float>(cv::contourArea(contour) / (length * width));
        if (!is_neighbor_weak_red_light_candidate(rect, length, width, fill, lights)) continue;
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
    for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float length = std::max(rect.size.width, rect.size.height);
        float width = std::min(rect.size.width, rect.size.height);
        if (width <= 0.0f) continue;
        float fill = static_cast<float>(cv::contourArea(contour) / (length * width));
        if (!is_neighbor_thin_light_candidate(rect, length, width, fill, lights)) continue;
        LightBar light;
        light.center = rect.center;
        light.length = length;
        light.width = width;
        light.fillRatio = fill;
        light.rect = rect;
        lights.push_back(light);
        cv::Point2f vertices[4]; rect.points(vertices);
        for (int i = 0; i < 4; ++i)
            cv::line(lightMask, vertices[i], vertices[(i + 1) % 4],
                     cv::Scalar(0, 0, 255), 1);
    }
    for (const auto& contour : contours) {
        cv::RotatedRect rect = cv::minAreaRect(contour);
        float length = std::max(rect.size.width, rect.size.height);
        float width = std::min(rect.size.width, rect.size.height);
        if (width <= 0.0f) continue;
        float fill = static_cast<float>(cv::contourArea(contour) / (length * width));
        if (!is_compact_light_candidate(rect, length, width, fill, lights)) continue;
        LightBar light;
        light.center = rect.center;
        light.length = length;
        light.width = width;
        light.fillRatio = fill;
        light.redOnly = true;
        light.rect = rect;
        lights.push_back(light);
        cv::Point2f vertices[4];
        rect.points(vertices);
        for (int i = 0; i < 4; ++i)
            cv::line(lightMask, vertices[i], vertices[(i + 1) % 4],
                     cv::Scalar(0, 0, 255), 1);
    }
}

inline void preview_detect_dual(const cv::Mat& mask, cv::Mat& lightMask,
                                std::vector<LightBar>& lights, const PreviewParams& p)
{
    cv::Mat blueMask, redMask, blueDraw, redDraw;
    cv::compare(mask, 255, blueMask, cv::CMP_EQ);
    cv::compare(mask, 128, redMask, cv::CMP_EQ);
    std::vector<LightBar> blueLights, redLights;
    preview_detect(blueMask, blueDraw, blueLights, p);
    preview_detect(redMask, redDraw, redLights, p, true);
    for (auto& light : redLights) light.isRed = true;
    lights = std::move(blueLights);
    lights.insert(lights.end(), redLights.begin(), redLights.end());
    cv::bitwise_or(blueDraw, redDraw, lightMask);
}

inline void preview_match(const cv::Mat& lightMask, cv::Mat& lightMatch,
                          const std::vector<LightBar>& lights,
                          std::vector<cv::RotatedRect>& armorRects,
                          const PreviewParams& p,
                          const std::vector<cv::RotatedRect>& previousArmorRects,
                          const std::vector<cv::RotatedRect>& olderArmorRects)
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
            if (lights[i].isRed != lights[j].isRed) continue;
            if (lights[i].redOnly || lights[j].redOnly) {
                cv::RotatedRect compactRect;
                double compactLoss;
                if (make_compact_neighbor_pair(lights[i], lights[j], compactRect, compactLoss))
                    candidates.push_back({i, j, compactLoss, compactRect});
                continue;
            }
            float lengthRatio = std::min(lights[i].length, lights[j].length) /
                                std::max(lights[i].length, lights[j].length);
            const float shorterLength = std::min(lights[i].length, lights[j].length);
            const float longerLength = std::max(lights[i].length, lights[j].length);
            const bool redShortLongPair = lights[i].isRed && shorterLength >= 10.0f &&
                shorterLength <= 19.0f && longerLength >= 20.0f &&
                longerLength <= 35.0f && lengthRatio >= 0.39f;
            if (lengthRatio < (lights[i].isRed &&
                               std::max(lights[i].length, lights[j].length) < 12.0f
                                   ? 0.44f : p.pairLength / 100.0f) &&
                !redShortLongPair) continue;
            float averageLength = (lights[i].length + lights[j].length) / 2.0f;
            cv::Point2f a = preview_long_direction(lights[i].rect);
            cv::Point2f b = preview_long_direction(lights[j].rect);
            if (a.dot(a) == 0.0f || b.dot(b) == 0.0f) continue;
            float similarity = std::abs(a.dot(b));
            cv::Point2f delta = lights[j].center - lights[i].center;
            float alongA = std::abs(delta.dot(a));
            float alongB = std::abs(delta.dot(b));
            cv::Point2f pairDirection = alongA <= alongB ? a : b;
            float along = std::min(alongA, alongB) / averageLength;
            float side = std::abs(pairDirection.x * delta.y -
                                  pairDirection.y * delta.x) / averageLength;
            if (lengthRatio < 0.46f &&
                !(averageLength < 10.0f && side > p.maxSide / 100.0f) &&
                !(averageLength < 12.0f && side <= 1.6f) &&
                !(lights[i].isRed && averageLength < 12.0f &&
                  lengthRatio >= 0.44f && side <= 5.8f) &&
                !redShortLongPair)
                continue;
            cv::Point2f first[4], second[4];
            lights[i].rect.points(first);
            lights[j].rect.points(second);
            std::vector<cv::Point2f> points;
            for (int k = 0; k < 4; ++k) {
                points.push_back(first[k]);
                points.push_back(second[k]);
            }
            cv::RotatedRect armorRect = cv::minAreaRect(points);
            bool matchesPrevious = matches_previous_pair(
                armorRect, previousArmorRects, olderArmorRects,
                p.previousLength / 100.0f, p.previousDistance / 100.0f);
            float maxSide = averageLength >= 12.0f && matchesPrevious ? 6.0f :
                            lights[i].isRed && averageLength < 12.0f ? 5.8f :
                            averageLength < 10.0f && lengthRatio < 0.5f
                                ? 5.0f : p.maxSide / 100.0f;
            float minSide = matchesPrevious || averageLength >= 10.5f
                                ? 0.70f : p.minSide / 100.0f;
            if (side > maxSide || side < minSide)
                continue;
            const bool normalGeometry =
                similarity >= p.pairDirection / 100.0f &&
                along <= (side >= 3.7f ? 1.10f : p.maxAlong / 100.0f);
            const bool continuedGeometry =
                matchesPrevious && similarity >= (averageLength < 10.0f ? 0.65f : 0.75f) &&
                along <= (side >= 3.7f ? 1.10f : 1.50f);
            if (!normalGeometry && !continuedGeometry) continue;
            double loss = along * p.alongWeight +
                          std::abs(side - p.targetSide / 100.0f) * p.sideWeight +
                          std::abs(similarity - 1.0f) * p.directionWeight +
                          std::abs(lengthRatio - 1.0f) * p.lengthWeight;
            // 与正式匹配一致：短灯条的过窄配对降低优先级。
            if (averageLength < 8.0f && side < 2.2f) loss += 9.0f;
            else if (side < 1.6f && averageLength < 11.5f) loss += 4.5f;
            if (matchesPrevious)
                loss -= p.continuityBonus / 100.0f +
                        (averageLength >= 12.0f ? 4.0f : 0.0f) +
                        (side < 1.2f && averageLength >= 13.0f ? 1.0f : 0.0f);
            // 同一高度若有灯条夹在两端之间，优先选择相邻灯条配对。
            if (averageLength >= 10.0f) {
                float left = std::min(lights[i].center.x, lights[j].center.x);
                float right = std::max(lights[i].center.x, lights[j].center.x);
                for (std::size_t mid = 0; mid < lights.size(); ++mid) {
                    if (mid == i || mid == j || lights[mid].redOnly) continue;
                    if (lights[mid].center.x > left + 2.0f &&
                        lights[mid].center.x < right - 2.0f &&
                        std::abs(lights[mid].center.y - armorRect.center.y) < averageLength * 2.0f)
                        loss += 6.0;
                }
            }
            candidates.push_back({i, j, loss, armorRect});
        }
    }
    reward_lower_pair(candidates, lights);
    reward_previous_triplet_side(candidates, lights.size(), previousArmorRects);
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
        [](const Candidate& candidate) {
            const float area = candidate.rect.size.area();
            return area >= 4500.0f ||
                   (area >= 3200.0f && candidate.loss > 2.0) ||
                   (area >= 2500.0f && area < 3200.0f && candidate.loss > 6.0);
        }), candidates.end());
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) { return a.loss < b.loss; });
    for (std::size_t index : select_unique_pairs(candidates, lights.size())) {
        const Candidate& candidate = candidates[index];
        armorRects.push_back(candidate.rect);
        cv::Point2f vertices[4];
        candidate.rect.points(vertices);
        for (int i = 0; i < 4; ++i)
            cv::line(lightMatch, vertices[i], vertices[(i + 1) % 4],
                     cv::Scalar(0, 255, 255), 2);
    }
}
