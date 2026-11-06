#include "main.h"
#include <algorithm>
void invertColor(unsigned char* pixelData)
{
    double b = pixelData[0] / 255.0;
    double g = pixelData[1] / 255.0;
    double r = pixelData[2] / 255.0;
    double maximum = std::max(r, std::max(g, b));
    double minimum = std::min(r, std::min(g, b));
    double H;
    if (maximum == minimum)
    {
        H = 0;
    }
    else if (minimum == b)
    {
        H = 60 * (g - r) / (maximum - minimum) + 60;
    }
    else if (minimum == r)
    {
        H = 60 * (b - g) / (maximum - minimum) + 180;
    }
    else if (minimum == g)
    {
        H = 60 * (r - b) / (maximum - minimum) + 300;
    }
    double S = maximum - minimum;
    double V = maximum;
    H+= 180;
    if (H>= 360)
    {
        H -= 360;
    }
    double H_prime = H / 60.0;
    double C = S;
    double X = C * (1 - std::abs(std::fmod(H_prime, 2) - 1));
    double m = V - C;
    double r1, g1, b1;
    if (0 <= H_prime && H_prime < 1)
    {
        r1 = C;
        g1 = X;
        b1 = 0;
    }
    else if (1 <= H_prime && H_prime < 2)
    {
        r1 = X;
        g1 = C;
        b1 = 0;
    }
    else if (2 <= H_prime && H_prime < 3)
    {
        r1 = 0;
        g1 = C;
        b1 = X;
    }
    else if (3 <= H_prime && H_prime < 4)
    {
        r1 = 0;
        g1 = X;
        b1 = C;
    }
    else if (4 <= H_prime && H_prime < 5)
    {
        r1 = X;
        g1 = 0;
        b1 = C;
    }
    else
    {
        r1 = C;
        g1 = 0;
        b1 = X;
    }
    pixelData[0] = static_cast<unsigned char>((b1 + m) * 255);
    pixelData[1] = static_cast<unsigned char>((g1 + m) * 255);
    pixelData[2] = static_cast<unsigned char>((r1 + m) * 255);
}
void repeat_invertColor(cv::Mat& image)
{
    //轮询
   for(int i=0;i<image.cols*image.rows;i++)
    {
    int row=i/image.cols;
    int col=i%image.cols;
    unsigned char* rowData;
    rowData=image.data+row*image.step[0];
    unsigned char* pixelData;
    pixelData=rowData+col*3;
    invertColor(pixelData);
    }
}

