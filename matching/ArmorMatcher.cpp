#include "ArmorMatcher.h"
#include <algorithm>  // std::min、std::max
#include <cmath>  // std::abs 的浮点数版本
#include <opencv2/imgproc.hpp>  // cv::line

void armor_match(const cv::Mat& lightMask, cv::Mat& lightMatch,const std::vector<LightBar>& lights)
{
lightMatch = lightMask.clone();
for (std::size_t i = 0; i < lights.size(); ++i)// 遍历每个灯条
{
    for (std::size_t j = i + 1; j < lights.size(); ++j)
    {
        float shorterLength = std::min(lights[i].length, lights[j].length);
        float longerLength  = std::max(lights[i].length, lights[j].length);
        float lengthRatio   = shorterLength / longerLength;
        // 排除灯条长度差异过大的情况
        if (lengthRatio < 0.7f)
        {
            continue;
        }

        // 平均长度
        float averageLength = (lights[i].length + lights[j].length) / 2.0f;
        
        // 取第一根灯条旋转矩形的四个顶点
        cv::Point2f vertices[4];
        lights[i].rect.points(vertices);

// 两条相邻边：其中一条是长边
        cv::Point2f edgeA = vertices[1] - vertices[0];
        cv::Point2f edgeB = vertices[2] - vertices[1];

// dot(自身) 得到边长的平方，比较后选出长边
        float edgeALengthSquared = edgeA.x * edgeA.x + edgeA.y * edgeA.y;
        float edgeBLengthSquared = edgeB.x * edgeB.x + edgeB.y * edgeB.y;
        cv::Point2f longEdge;
        if (edgeALengthSquared >= edgeBLengthSquared)
        {
           longEdge = edgeA;
        }
        else
        {
           longEdge = edgeB;
        }

// 把长边变成长度为 1 的方向向量
        float edgeLength = std::sqrt(longEdge.x * longEdge.x + longEdge.y * longEdge.y);
         if (edgeLength <= 0.0f)
        {
            continue;
        }
        //不太会，抄的公式
        cv::Point2f direction = longEdge * (1.0f / edgeLength);
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
       if (alongRatio > 0.5f)
       {
            continue;
       }

// 侧向间距太小或太大：排除近似共线及距离过远的组合
       if (sideRatio < 0.5f || sideRatio > 2.3f)
        {
            continue;
        }
        // 如果满足条件，则认为这两个灯条可以组成装甲板，绘制连接线，颜色为黄色，厚度为2
        cv::line(lightMatch,lights[i].center,lights[j].center, cv::Scalar(0, 255, 255),2);
        {
            continue;
        }
    }  
}
}




