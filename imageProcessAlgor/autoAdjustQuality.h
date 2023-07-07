#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
extern "C"
{
    enum QualityType
    {
        FOCUS,
        BRIGHTNESS
    };
    typedef int (*CaptureImage)(char *, int &, int);

    /**
     * @brief Search target from source image and return the location of the matched target image.
     * @param image 		source image data buffer encoded with Base64
     * @param imageSize		buffer size
     * @param type          focus quality or brightness quality. Default value is focus quality.
     * @return matched quality if successful. otherwise return non zero if failed.
     */
    __declspec(dllexport) float ImageQuality(char *image, int imageSize, QualityType type = QualityType::FOCUS);

    /**
     * @brief calculate the quality on specified focus value
     * @param minPosition               the minimum value of focus/brightness
     * @param maxPosition               the maximum value of focus/brightness
     * @param startPosition             the begining search position
     * @param step                      the step for next position
     * @param captureImage              the function to capture the image
     * @return return 0 if successful, otherwise non-zero will return.
     */
    __declspec(dllexport) int AutoAdjust(int minPosition,
                                         int maxPosition,
                                         int startPosition,
                                         int step,
                                         CaptureImage captureImage,
                                         QualityType type = QualityType::FOCUS);

    __declspec(dllexport) float QueryQuality(int position, CaptureImage captureImage, QualityType type = QualityType::FOCUS);
    __declspec(dllexport) float FocusQuality(cv::Mat &image);
    __declspec(dllexport) float BrightQuality(cv::Mat &image);

    int BisectionSearch(int start, int end, int user_step, CaptureImage captureImage, QualityType type = QualityType::FOCUS);

    float StatSharpnessGradient(cv::Mat &image);
    float StatSharpnessTenengrad(cv::Mat &image, const int threshold = 500);
    float StatSharpnessLaplacian(cv::Mat &image);
    float StatSharpnessVariance(cv::Mat &img);

    float StatBrightnessMean(cv::Mat &image);
    float StatBrightnessRMS(cv::Mat &img);
    float StatBrightnessFormula(cv::Mat &image);
    float StatBrightnessRMSFormula(cv::Mat &image);
}