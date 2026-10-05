#include "ArmorMatcher.h"
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

void armor_match(const cv::Mat& lightMask, cv::Mat& lightMatch,const std::vector<LightBar>& lights,std::vector<cv::RotatedRect>& armorRects)
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
        float shorterLength = std::min(lights[i].length, lights[j].length);
        float longerLength  = std::max(lights[i].length, lights[j].length);
        float lengthRatio   = shorterLength / longerLength;
        // 排除灯条长度差异过大的情况
        if (lengthRatio < 0.5f)
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

        // 排除长边方向差异过大的组合。
        float directionSimilarity = std::abs(direction.x * secondDirection.x + direction.y * secondDirection.y);
        if (directionSimilarity < 0.9f)
        {
            continue;
        }
// 从第一根灯条中心指向第二根灯条中心
        cv::Point2f centerDelta = lights[j].center - lights[i].center;
// 沿长边方向的错位：点积得到投影长度
        float alongDistance = std::abs(centerDelta.dot(direction));
// 垂直长边方向的间距：二维叉积的绝对值
        float sideDistance = std::abs(direction.x * centerDelta.y -direction.y * centerDelta.x);
// 换算成相对于平均灯条长度的比例
       float alongRatio = alongDistance / averageLength;
       float sideRatio  = sideDistance / averageLength;
        
// 沿长边方向错位太大，跳过当前组合
       if (alongRatio > 0.9f)
       {
            continue;
       }

// 侧向间距太小或太大：排除近似共线及距离过远的组合
       if (sideRatio < 1.65f || sideRatio > 4.6f)
        {
            continue;
        }
        
    double losescore=0;
       losescore=alongRatio*1+abs(sideRatio-2.49)+abs(directionSimilarity-1)*5+abs(lengthRatio-1)*2 ;
        // 满足条件后，生成装甲板候选矩形。
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
        pairCandidates.push_back({i, j, losescore, armorRect});
    }  
}
// 对 pairCandidates 按照损失分数进行升序排序，损失分数越低的组合越优先考虑
// 使用 lambda 表达式作为比较函数，比较两个 PairCandidate 的 loss 成员
std::sort(pairCandidates.begin(), pairCandidates.end(),
          [](const PairCandidate& a, const PairCandidate& b) 
          {
              return a.loss < b.loss;
          });
// 使用一个布尔向量记录每个灯条是否已经被使用，初始值为 false
std::vector<bool> used(lights.size(), false);
for (const PairCandidate& candidate : pairCandidates)
{
    // 如果任意一根灯条已经被使用，则跳过当前组合
    if (used[candidate.first] || used[candidate.second])
    {
        continue;
    }

    // 标记当前组合的两根灯条为已使用
    used[candidate.first] = true;
    used[candidate.second] = true;
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
