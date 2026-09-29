#include "InputHandle.h"
#include <vector>
#include <cmath>
#include <iostream>
void draw_display(const cv::Mat& lightMatch, const std::vector<cv::RotatedRect>& armorRects, cv::Mat& handleImage)
{
    handleImage = lightMatch.clone();
    for (const auto& armorRect : armorRects)
{
    cv::Point2f armorPoints[4];
    armorRect.points(armorPoints);

    for (int k = 0; k < 4; ++k)
    {
        cv::line(handleImage,
                 armorPoints[k],
                 armorPoints[(k + 1) % 4],
                 cv::Scalar(0, 255, 255), 2);
    }
}
}



bool save_video(cv::VideoWriter& writer, const cv::Mat& image, double fps,
                const std::string& outputPath)
{
    // 检查图像是否为空
    if (image.empty())
    {
        std::cerr << "没有可保存的图像\n";
        return false;
    }
    
    // 如果视频写入器尚未打开，则尝试打开它
    if (!writer.isOpened())
    {
        // 检查 fps 是否为有限值且大于 0。如果 fps 无效，则设置为默认值 30.0
        if (!std::isfinite(fps) || fps <= 0.0)
        {
            fps = 30.0;
        }
        // 尝试打开视频写入器
        writer.open(outputPath, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),fps, image.size());
        // 检查视频写入器是否成功打开
        if (!writer.isOpened())
        {
            std::cerr << "无法创建结果视频：" << outputPath << '\n';
            return false;
        }
    }
    
// 将图像写入视频
    writer.write(image);
    return true;
}
