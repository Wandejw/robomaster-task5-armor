#pragma once

#include <opencv2/core.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
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

// 仅在三根灯条形成两种竞争配对时，延续上一帧所选的左右一侧。
// 用横向位置判断配对侧，允许目标整体快速上下移动。
template <class Candidate>
void reward_previous_triplet_side(std::vector<Candidate>& candidates,
                                  std::size_t lightCount,
                                  const std::vector<cv::RotatedRect>& previousRects)
{
    std::vector<std::size_t> parent(lightCount);
    for (std::size_t k = 0; k < lightCount; ++k) parent[k] = k;
    auto root = [&](std::size_t k) {
        while (parent[k] != k) k = parent[k];
        return k;
    };
    for (const auto& candidate : candidates)
        parent[root(candidate.first)] = root(candidate.second);
    std::vector<int> componentSize(lightCount, 0);
    for (std::size_t k = 0; k < lightCount; ++k) ++componentSize[root(k)];

    for (auto& candidate : candidates) {
        if (componentSize[root(candidate.first)] != 3) continue;
        float currentLength = std::max(candidate.rect.size.width,
                                       candidate.rect.size.height);
        for (const auto& previous : previousRects) {
            float previousLength = std::max(previous.size.width,
                                            previous.size.height);
            if (std::min(currentLength, previousLength) /
                    std::max(currentLength, previousLength) < 0.6f ||
                std::abs(candidate.rect.center.x - previous.center.x) > 12.0f ||
                std::abs(candidate.rect.center.y - previous.center.y) > 90.0f)
                continue;
            bool competingPair = false;
            for (const auto& other : candidates) {
                if (&other == &candidate) continue;
                bool sharesBar = candidate.first == other.first ||
                                 candidate.first == other.second ||
                                 candidate.second == other.first ||
                                 candidate.second == other.second;
                if (sharesBar &&
                    std::abs(other.rect.center.y - candidate.rect.center.y) < 20.0f &&
                    std::abs(other.rect.center.x - candidate.rect.center.x) >= 15.0f &&
                    std::abs(other.rect.center.x - previous.center.x) >=
                        std::abs(candidate.rect.center.x - previous.center.x) + 12.0f) {
                    competingPair = true;
                    break;
                }
            }
            if (competingPair) { candidate.loss -= 6.5; break; }
        }
    }
}

// 三灯条的左右两种候选同时存在时，优先选择位置较低的一侧。
template <class Candidate, class Light>
void reward_lower_pair(std::vector<Candidate>& candidates,
                       const std::vector<Light>& lights)
{
    std::vector<std::size_t> parent(lights.size());
    for (std::size_t k = 0; k < lights.size(); ++k) parent[k] = k;
    auto root = [&](std::size_t k) {
        while (parent[k] != k) k = parent[k];
        return k;
    };
    for (const auto& candidate : candidates)
        parent[root(candidate.first)] = root(candidate.second);
    std::vector<int> groupSize(lights.size(), 0);
    for (std::size_t k = 0; k < lights.size(); ++k) ++groupSize[root(k)];

    for (auto& candidate : candidates) {
        if (groupSize[root(candidate.first)] == 3) {
            std::vector<std::size_t> bars;
            for (std::size_t k = 0; k < lights.size(); ++k)
                if (root(k) == root(candidate.first)) bars.push_back(k);
            std::sort(bars.begin(), bars.end(), [&](std::size_t a, std::size_t b) {
                return lights[a].center.x < lights[b].center.x;
            });
            bool leftPair = (candidate.first == bars[0] && candidate.second == bars[1]) ||
                            (candidate.first == bars[1] && candidate.second == bars[0]);
            float leftY = (lights[bars[0]].center.y + lights[bars[1]].center.y) / 2.0f;
            float rightY = (lights[bars[1]].center.y + lights[bars[2]].center.y) / 2.0f;
            if (leftPair && leftY >= rightY - 1.0f && leftY - rightY <= 2.0f)
                candidate.loss -= 16.0;
        }
    }
}
