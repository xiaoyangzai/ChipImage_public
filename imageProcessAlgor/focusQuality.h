#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
extern "C"
{
    typedef int (*CaptureImage)(char *, int &, int);
    /**
     * @brief calculate the quality on specified focus value
     * @param min_focus                 the minimum value of focus
     * @param max_focus                 the maximum value of focus
     * @param step                      the value for each step to adjust focus
     * @param captureImage              the function to capture the image
     * @return return 0 if successful, otherwise non-zero will return.
     */
    __declspec(dllexport) int AutoFocus(int min_focus,
                                        int max_focus,
                                        int step,
                                        CaptureImage captureImage);
    __declspec(dllexport) int FocusQuality(cv::Mat& image);
    // float StatSharpnessGradient(cv::Mat& image);
    float StatSharpnessTenengrad(cv::Mat& image, int threshold);
    float Stat_sharpness_Laplacian(cv::Mat& image);
    float StatSharpnessVariance(cv::Mat& img);
}