#pragma once

#include "DetectionTypes.h"
#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <vector>

inline bool is_neighbor_weak_red_light_candidate(
    const cv::RotatedRect& rect, float length, float width, float fill,
    const std::vector<LightBar>& regularLights)
{
    if (fill < 0.28f || fill >= 0.33f || length < 10.0f || length > 22.0f ||
        width < 2.0f || width > 4.0f || length / width > 10.01f)
        return false;
    cv::Point2f vertices[4];
    rect.points(vertices);
    cv::Point2f a = vertices[1] - vertices[0];
    cv::Point2f b = vertices[2] - vertices[1];
    cv::Point2f direction = a.dot(a) >= b.dot(b) ? a : b;
    if (std::abs(direction.x) > std::abs(direction.y)) return false;
    for (const auto& light : regularLights) {
        if (light.redOnly || light.length < 20.0f || light.length > 35.0f) continue;
        float dx = std::abs(rect.center.x - light.center.x);
        float dy = std::abs(rect.center.y - light.center.y);
        if (dx >= 1.6f * light.length && dx <= 3.2f * light.length &&
            dy <= 1.8f * light.length)
            return true;
    }
    return false;
}

// 只补画靠近真实灯条的小型圆形蓝点，避免整幅画面的圆形装饰都变成候选。
inline bool is_compact_light_candidate(
    const cv::RotatedRect& rect, float length, float width, float fill,
    const std::vector<LightBar>& regularLights)
{
    if (length < 4.0f || length > 7.1f || width < 2.0f ||
        length / width >= 2.01f || fill < 0.57f) return false;

    for (const LightBar& light : regularLights) {
        if (light.redOnly) continue;
        float dx = std::abs(rect.center.x - light.center.x);
        float dy = std::abs(rect.center.y - light.center.y);
        if (dx >= 10.0f && dx <= 65.0f && dy <= 20.0f) return true;
    }
    return false;
}

// 仅补回紧邻已确认灯条的细蓝线，避免全局放宽长宽比带来噪声。
inline bool is_neighbor_thin_light_candidate(
    const cv::RotatedRect& rect, float length, float width, float fill,
    const std::vector<LightBar>& lights)
{
    if (length < 8.0f || length > 12.0f || width < 0.9f ||
        length / width <= 10.01f || length / width > 11.5f || fill < 0.70f)
        return false;
    cv::Point2f points[4];
    rect.points(points);
    cv::Point2f a = points[1] - points[0], b = points[2] - points[1];
    cv::Point2f direction = a.dot(a) >= b.dot(b) ? a : b;
    if (std::abs(direction.x) > std::abs(direction.y)) return false;
    for (const auto& light : lights) {
        if (light.redOnly) continue;
        float dx = std::abs(rect.center.x - light.center.x);
        float dy = std::abs(rect.center.y - light.center.y);
        if (dx >= 8.0f && dx <= 18.0f && dy <= 10.0f) return true;
    }
    return false;
}

// 圆形蓝点平时只画红框；紧邻一根普通短灯条时允许组成候选黄框。
inline bool make_compact_neighbor_pair(const LightBar& first,
                                       const LightBar& second,
                                       cv::RotatedRect& pairRect,
                                       double& pairLoss)
{
    if (first.redOnly == second.redOnly) return false;
    const LightBar& dot = first.redOnly ? first : second;
    const LightBar& bar = first.redOnly ? second : first;
    float dx = std::abs(dot.center.x - bar.center.x);
    float dy = std::abs(dot.center.y - bar.center.y);
    if (dx < 21.0f || dx > 30.0f || dy > 5.5f ||
        bar.length < 9.5f || bar.length > 17.0f)
        return false;
    cv::Point2f a[4], b[4];
    first.rect.points(a);
    second.rect.points(b);
    std::vector<cv::Point2f> points;
    for (int k = 0; k < 4; ++k) {
        points.push_back(a[k]);
        points.push_back(b[k]);
    }
    pairRect = cv::minAreaRect(points);
    pairLoss = 3.0 + std::abs(dx / bar.length - 2.0f) * 2.0 + dy / bar.length;
    return true;
}
