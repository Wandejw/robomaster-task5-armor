#pragma once

#include <opencv2/core.hpp>
#include <algorithm>
#include <vector>

// 用前两帧的位置估计当前框；只有尺寸和预测位置都接近才延续配对。
inline bool matches_previous_pair(
    const cv::RotatedRect& current,
    const std::vector<cv::RotatedRect>& previousRects,
    const std::vector<cv::RotatedRect>& olderRects,
    float minSizeRatio, float maxDistanceRatio)
{
    float currentLength = std::max(current.size.width, current.size.height);
    for (const cv::RotatedRect& previous : previousRects) {
        float previousLength = std::max(previous.size.width, previous.size.height);
        float sizeRatio = std::min(currentLength, previousLength) /
                          std::max(currentLength, previousLength);
        if (sizeRatio < minSizeRatio) continue;

        cv::Point2f predictedCenter = previous.center;
        float historyDistance = 1.5f * previousLength;
        float nearestDistanceSquared = historyDistance * historyDistance;
        for (const cv::RotatedRect& older : olderRects) {
            float olderLength = std::max(older.size.width, older.size.height);
            float olderSizeRatio = std::min(previousLength, olderLength) /
                                   std::max(previousLength, olderLength);
            if (olderSizeRatio < minSizeRatio) continue;
            cv::Point2f movement = previous.center - older.center;
            float distanceSquared = movement.dot(movement);
            if (distanceSquared <= nearestDistanceSquared) {
                nearestDistanceSquared = distanceSquared;
                predictedCenter = previous.center + movement;
            }
        }

        cv::Point2f delta = current.center - predictedCenter;
        float maxDistance = maxDistanceRatio * currentLength;
        if (delta.dot(delta) <= maxDistance * maxDistance) return true;
    }
    return false;
}
