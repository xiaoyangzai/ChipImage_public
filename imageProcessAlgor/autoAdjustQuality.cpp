#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "autoAdjustQuality.h"
using namespace cv;
using namespace std;
#define MEM_MAX_SIZE 1024 * 1024 * 15
static char *g_dynamicMem = NULL;

extern "C"
{

    __declspec(dllexport) int AutoAdjust(int minPosition, int maxPosition, int startPosition, CaptureImage captureImage, QualityType type)
    {
        float quality = -1.0;
        char msg[256] = "";
        if (captureImage == NULL)
        {
            sprintf_s(msg, sizeof(msg) - strlen(msg), "Empty image capture handler. Please check again!\n");
            DebugPrint(msg);
            return -1;
        }
        if (minPosition > maxPosition || startPosition < minPosition || startPosition > maxPosition)
        {
            sprintf_s(msg, sizeof(msg) - strlen(msg), "Bad focus position. Please check again!\n");
            DebugPrint(msg);
            return -1;
        }
        if (!g_dynamicMem)
            g_dynamicMem = (char *)malloc(MEM_MAX_SIZE * sizeof(char));
        int optimumPosition = -1;
        sprintf_s(msg, sizeof(msg) - strlen(msg), "[AutoAdjust] Max: %d, Min: %d, Step: %d\n", minPosition, maxPosition);

        auto startSearch = std::chrono::high_resolution_clock::now();
        optimumPosition = BisectionSearch(minPosition, maxPosition, captureImage, type);
        auto endSearch = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endSearch - startSearch).count();
        quality = QueryQuality(optimumPosition, captureImage, type);

        sprintf_s(msg, sizeof(msg) - strlen(msg), "[AutoAdjust] Duration of searching optimum focus: %lld ms\n\tOptimum position: %d\n\tQuality: %.3f\n", duration, optimumPosition, quality);
        DebugPrint(msg);
        free(g_dynamicMem);
        return 0;
    }

    __declspec(dllexport) float QueryQuality(int position, CaptureImage captureImage, QualityType type)
    {
        if (!g_dynamicMem)
            g_dynamicMem = (char *)malloc(MEM_MAX_SIZE * sizeof(char));
        if (captureImage == NULL)
            return -1.0;
        int length = -1;
        if (captureImage(g_dynamicMem, length, position) < 0)
            return -1.0;
        std::string imageData = Base64Decoder(g_dynamicMem, length);
        std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
        cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
        if (type == QualityType::FOCUS)
            return FocusQuality(imageMat);
        if (type == QualityType::BRIGHTNESS)
            return BrightQuality(imageMat);
        return -1;
    }

    __declspec(dllexport) float FocusQuality(cv::Mat &image)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based with the current focus setting
        quality = StatSharpnessTenengrad(image);
        std::string ret = "Image focus quality: " + std::to_string(quality);
        DebugPrint(ret.c_str());
        return quality;
    }

    __declspec(dllexport) float BrightQuality(cv::Mat &image)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based with the current focus setting
        quality = StatBrightnessRMS(image);
        std::string ret = "Image focus quality: " + std::to_string(quality);
        DebugPrint(ret.c_str());
        return quality;
    }

    int BisectionSearch(int start, int end, CaptureImage captureImage, QualityType type)
    {
        if (start > end)
            return -1;
        static int step = (end - start) / 10;
        static int round = 1;
        step = step / round;
        step = step > 0 ? step : 1;
        round *= 2;

        int mid = start + (end - start) / 2;
        float midQuality = QueryQuality(mid, captureImage, type);
        float frontQuality = QueryQuality(mid - step, captureImage, type);
        float behindQuality = QueryQuality(mid + step, captureImage, type);

        if (midQuality >= frontQuality && midQuality >= behindQuality)
            return mid;
        else if (midQuality < frontQuality)
            return BisectionSearch(start, mid - 1, captureImage, type);
        else
            return BisectionSearch(mid + 1, end, captureImage, type);
    }

    /**********
     * Functions to calculate focus quality
     */
    float StatSharpnessGradient(cv::Mat &image)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on gradient
        cv::Mat gradient_x, gradient_y;
        cv::Sobel(image, gradient_x, CV_64F, 1, 0, 3);
        cv::Sobel(image, gradient_y, CV_64F, 0, 1, 3);
        cv::Mat gradient_magnitude = cv::Mat(image.size(), CV_64F);
        cv::sqrt(gradient_x.mul(gradient_x) + gradient_y.mul(gradient_y), gradient_magnitude);
        quality = cv::mean(gradient_magnitude.mul(gradient_magnitude))[0];
        return quality;
    }

    float StatSharpnessTenengrad(cv::Mat &img, const int threshold)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on Tenengrad
        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::Mat gx, gy;
        cv::Sobel(img, gx, CV_64F, 1, 0, 3);
        cv::Sobel(img, gy, CV_64F, 0, 1, 3);
        cv::add(gx.mul(gx), gy.mul(gy), gx);
        cv::compare(gx, threshold, gx, cv::CMP_GT);
        gx.convertTo(gx, CV_64F, 1.0 / 255);
        quality = cv::mean(gx)[0];
        return quality;
    }

    float StatSharpnessLaplacian(cv::Mat &img)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on Laplacian
        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::Laplacian(img, img, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(img, mu, sigma);
        quality = sigma[0] * sigma[0];
        return quality;
    }

    float StatSharpnessVariance(cv::Mat &img)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on Variance
        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(img, mu, sigma);
        quality = sigma[0] * sigma[0];
        return quality;
    }

    /**********
     * Functions to calculate brightness quality
     */
    float StatBrightnessMean(cv::Mat &img)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on Mean
        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        quality = cv::mean(img)[0];
        return quality;
    }

    float StatBrightnessRMS(cv::Mat &img)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on RMS
        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::multiply(img, img, img);
        quality = std::sqrt(cv::mean(img)[0]);
        return quality;
    }

    float StatBrightnessFormula(cv::Mat &img)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on Formula
        if (img.channels() == 1)
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
        img.convertTo(img, CV_64F);
        cv::Scalar rgb = cv::mean(img);
        quality = std::sqrt((0.241 * rgb[2] * rgb[2] + 0.691 * rgb[1] * rgb[1] + 0.068 * rgb[0] * rgb[0]));
        return quality;
    }

    float StatBrightnessRMSFormula(cv::Mat &img)
    {
        float quality = -1.0;
        // TODO: calculate the image quality based on RMS Formula
        if (img.channels() == 1)
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
        img.convertTo(img, CV_64F);
        cv::multiply(img, img, img);
        cv::Scalar rms = cv::mean(img);
        quality = std::sqrt((0.241 * rms[2] + 0.691 * rms[1] + 0.068 * rms[0]));
        return quality;
    }
}
