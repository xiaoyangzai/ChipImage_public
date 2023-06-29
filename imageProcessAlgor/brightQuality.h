#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
extern "C"
{
    typedef int (*CaptureImage)(char *, int &, int);
    /**
     * @brief calculate the quality on specified focus value
     * @param minBright                 the minimum value of brightness 
     * @param maxBright                 the maximum value of brightness 
     * @param step                      the value for each step to adjust brightness
     * @param captureImage              the function to capture the image
     * @return return 0 if successful, otherwise non-zero will return.
     */
    __declspec(dllexport) int AutoBright(int minBright,
                                        int maxBright,
                                        int step,
                                        CaptureImage captureImage);
    __declspec(dllexport) float BrightQuality(cv::Mat& image);
    float StatBrightnessMean(cv::Mat& image); 
    float StatBrightnessRMS(cv::Mat& image); 
    float StatBrightnessFormula(cv::Mat& image); 
    float StatBrightnessRMSFormula(cv::Mat& image); 
}