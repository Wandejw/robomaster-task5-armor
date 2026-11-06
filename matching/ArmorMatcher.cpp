#include "ArmorMatcher.h"
#include "../common/CompactLightCandidate.h"
#include "../common/PairContinuity.h"
#include "../common/PairSelection.h"
#include <algorithm>  // std::min、std::max
#include <cmath>  // std::abs 的浮点数版本
#include <opencv2/imgproc.hpp>  // cv::line
#include <cstddef>// 该函数用于匹配灯条，生成装甲板候选矩形，并在 lightMatch 上绘制装甲板轮廓。
#include <vector>  
// 取得旋转矩形长边的单位方向，供两根灯条复用。
static cv::Point2f get_long_direction(const cv::RotatedRect& rect)
{
    // 获取旋转矩形的四个顶点
    cv::Point2f vertices[4];
    rect.points(vertices);
    // 计算两条边的向量
    cv::Point2f edgeA = vertices[1] - vertices[0];
    cv::Point2f edgeB = vertices[2] - vertices[1];
    // 计算两条边的长度平方
    float edgeALengthSquared = edgeA.x * edgeA.x + edgeA.y * edgeA.y;
    float edgeBLengthSquared = edgeB.x * edgeB.x + edgeB.y * edgeB.y;
    
    cv::Point2f longEdge;
    // 选择较长的边作为长边
    if (edgeALengthSquared >= edgeBLengthSquared)
    {
        longEdge = edgeA;
    }
    else
    {
        longEdge = edgeB;
    }
    // 计算长边的长度
    float edgeLength = std::sqrt(longEdge.x * longEdge.x + longEdge.y * longEdge.y);
    if (edgeLength <= 0.0f)
    {
        return cv::Point2f(0.0f, 0.0f);
    }
    // 归一化长边向量，得到单位方向
    cv::Point2f direction;
    direction.x = longEdge.x / edgeLength;
    direction.y = longEdge.y / edgeLength;
    return direction;
}

void armor_match(const cv::Mat& lightMask, cv::Mat& lightMatch,
                 const std::vector<LightBar>& lights,
                 std::vector<cv::RotatedRect>& armorRects,
                 const std::vector<cv::RotatedRect>& previousArmorRects,
                 const std::vector<cv::RotatedRect>& olderArmorRects)
{
// 每帧清空结果，避免上一帧的装甲板残留。
armorRects.clear();
lightMatch = lightMask.clone();

struct PairCandidate
{
    std::size_t first;std::size_t second;double loss;
    // 旋转矩形，表示装甲板候选区域
    cv::RotatedRect rect;
};

std::vector<PairCandidate> pairCandidates;
for (std::size_t i = 0; i < lights.size(); ++i)// 遍历每个灯条
{

    for (std::size_t j = i + 1; j < lights.size(); ++j)
    {
        if (lights[i].isRed != lights[j].isRed) continue;
        if (lights[i].redOnly || lights[j].redOnly)
        {
            cv::RotatedRect compactRect;
            double compactLoss;
            if (make_compact_neighbor_pair(lights[i], lights[j], compactRect, compactLoss))
                pairCandidates.push_back({i, j, compactLoss, compactRect});
            continue;
        }
        float shorterLength = std::min(lights[i].length, lights[j].length);
        float longerLength  = std::max(lights[i].length, lights[j].length);
        float lengthRatio   = shorterLength / longerLength;
        const bool redShortLongPair = lights[i].isRed && shorterLength >= 10.0f &&
            shorterLength <= 19.0f && longerLength >= 20.0f &&
            longerLength <= 35.0f && lengthRatio >= 0.39f;
        // 排除灯条长度差异过大的情况
        if (lengthRatio < (lights[i].isRed && longerLength < 12.0f ? 0.44f : 0.45f) &&
            !redShortLongPair)
        {
            continue;
        }

        // 平均长度
        float averageLength = (lights[i].length + lights[j].length) / 2.0f;
        // 获取两根灯条的长边方向
        cv::Point2f direction = get_long_direction(lights[i].rect);
        cv::Point2f secondDirection = get_long_direction(lights[j].rect);
        // 任意一根灯条的长边方向为零向量，则跳过当前组合
        if ((direction.x == 0.0f && direction.y == 0.0f) ||(secondDirection.x == 0.0f && secondDirection.y == 0.0f))
        {
            continue;
        }

        // 比较两根灯条长边方向。
        float directionSimilarity = std::abs(direction.x * secondDirection.x + direction.y * secondDirection.y);
// 从第一根灯条中心指向第二根灯条中心
        cv::Point2f centerDelta = lights[j].center - lights[i].center;
// 沿长边方向的错位：点积得到投影长度
        float alongFirst = std::abs(centerDelta.dot(direction));
        float alongSecond = std::abs(centerDelta.dot(secondDirection));
        cv::Point2f pairDirection = alongFirst <= alongSecond ? direction : secondDirection;
        float alongDistance = std::min(alongFirst, alongSecond);
// 垂直长边方向的间距：二维叉积的绝对值
        float sideDistance = std::abs(pairDirection.x * centerDelta.y -
                                      pairDirection.y * centerDelta.x);
// 换算成相对于平均灯条长度的比例
       float alongRatio = alongDistance / averageLength;
       float sideRatio  = sideDistance / averageLength;
        // 常规配对仍要求长度比 0.5；短灯条与较远的同伴允许放宽到 0.45。
        if (lengthRatio < 0.46f &&
            !(averageLength < 10.0f && sideRatio > 4.7f) &&
            !(averageLength < 12.0f && sideRatio <= 1.6f) &&
            !(lights[i].isRed && averageLength < 12.0f &&
              lengthRatio >= 0.44f && sideRatio <= 5.8f) &&
            !redShortLongPair)
        {
            continue;
        }
        
        // 先生成候选矩形，用上一帧位置判断是否允许较近的灯条继续配对。
        // 取两根灯条各自的四个顶点
       cv::Point2f firstPoints[4];
       cv::Point2f secondPoints[4];
       lights[i].rect.points(firstPoints);
       lights[j].rect.points(secondPoints);

        // 把两根灯条的 8 个顶点放进同一个列表
        std::vector<cv::Point2f> allPoints;
        for (int k = 0; k < 4; k++)
        {
        allPoints.push_back(firstPoints[k]);
        allPoints.push_back(secondPoints[k]);
        }

        // 求包住这 8 个点的最小旋转矩形
        // cv::minAreaRect 函数返回一个 cv::RotatedRect 对象，表示最小外接旋转矩形
       cv::RotatedRect armorRect = cv::minAreaRect(allPoints);
        bool matchesPrevious = matches_previous_pair(
            armorRect, previousArmorRects, olderArmorRects, 0.7f, 0.5f);
        // 横向间距超过上限，或新候选过近时，排除该组合。
        float maxSideRatio = averageLength >= 12.0f && matchesPrevious ? 6.0f :
                             lights[i].isRed && averageLength < 12.0f ? 5.8f :
                             averageLength < 10.0f && lengthRatio < 0.5f ? 5.0f : 4.7f;
        float minSideRatio = matchesPrevious || averageLength >= 10.5f ? 0.70f : 1.15f;
        if (sideRatio > maxSideRatio || sideRatio < minSideRatio)
        {
            continue;
        }
        // 间距较宽的候选若纵向错位明显，通常是跨目标的斜配对；延续奖励也不能放行。
        float maxAlongRatio = sideRatio >= 3.7f ? 1.10f : 1.70f;
        bool normalGeometry = directionSimilarity >= 0.88f && alongRatio <= maxAlongRatio;
        bool continuedGeometry = matchesPrevious &&
                                 directionSimilarity >= (averageLength < 10.0f ? 0.65f : 0.75f) &&
                                 alongRatio <= (sideRatio >= 3.7f ? 1.10f : 1.50f);
        if (!normalGeometry && !continuedGeometry)
        {
            continue;
        }
        double losescore = alongRatio * 3 + std::abs(sideRatio - 2.90f) * 3 +
                           std::abs(directionSimilarity - 1.0f) * 5 +
                           std::abs(lengthRatio - 1.0f) * 2;
        // 短灯条紧挨时容易被误当成一组；降低这种窄配对的优先级。
        if (averageLength < 8.0f && sideRatio < 2.2f) losescore += 9.0;
        else if (sideRatio < 1.6f && averageLength < 11.5f) losescore += 4.5;
        if (matchesPrevious)
            losescore -= 5.0 + (averageLength >= 12.0f ? 4.0 : 0.0) +
                         (sideRatio < 1.2f && averageLength >= 13.0f ? 1.0 : 0.0);
        // 同一高度若有灯条夹在两端之间，优先选择相邻灯条配对，减少交叉黄框。
        if (averageLength >= 10.0f) {
            float left = std::min(lights[i].center.x, lights[j].center.x);
            float right = std::max(lights[i].center.x, lights[j].center.x);
            for (std::size_t mid = 0; mid < lights.size(); ++mid) {
                if (mid == i || mid == j || lights[mid].redOnly) continue;
                if (lights[mid].center.x > left + 2.0f &&
                    lights[mid].center.x < right - 2.0f &&
                    std::abs(lights[mid].center.y - armorRect.center.y) < averageLength * 2.0f)
                    losescore += 6.0;
            }
        }
        pairCandidates.push_back({i, j, losescore, armorRect});
    }  
}
reward_lower_pair(pairCandidates, lights);
reward_previous_triplet_side(pairCandidates, lights.size(), previousArmorRects);
// 大框按配对损失分数筛选，超大框直接排除。
pairCandidates.erase(std::remove_if(pairCandidates.begin(), pairCandidates.end(),
    [](const PairCandidate& candidate) {
        const float area = candidate.rect.size.area();
        return area >= 4500.0f ||
               (area >= 3200.0f && candidate.loss > 2.0) ||
               (area >= 2500.0f && area < 3200.0f && candidate.loss > 6.0);
    }), pairCandidates.end());
// 对 pairCandidates 按照损失分数进行升序排序，损失分数越低的组合越优先考虑
// 使用 lambda 表达式作为比较函数，比较两个 PairCandidate 的 loss 成员
std::sort(pairCandidates.begin(), pairCandidates.end(),
          [](const PairCandidate& a, const PairCandidate& b) 
          {
              return a.loss < b.loss;
          });
for (std::size_t index : select_unique_pairs(pairCandidates, lights.size()))
{
    const PairCandidate& candidate = pairCandidates[index];
    // 将装甲板候选矩形加入结果列表 armorRects
    armorRects.push_back(candidate.rect);
// 绘制装甲板轮廓到 lightMatch 上
    cv::Point2f armorPoints[4];
    // 获取装甲板候选矩形的四个顶点
    candidate.rect.points(armorPoints);
    for (int k = 0; k < 4; ++k)
    {
        cv::line(lightMatch, armorPoints[k], armorPoints[(k + 1) % 4],
                 cv::Scalar(0, 255, 255), 2);
    }
}
}
