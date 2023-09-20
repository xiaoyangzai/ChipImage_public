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
static char* g_dynamicMem = NULL;
static bool isFocusFirstFlag = true;
static bool isBrightFirstFlag = true;

extern "C" {

__declspec(dllexport) float ImageQuality(char* image, int imageSize, QualityType type) {
    char msg[256] = "";
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "[INFO] Calling ImageQuality()....\n");
    float quality = -1.0;
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat srcImage = imdecode(decodedImage, cv::IMREAD_COLOR);
    if (!srcImage.data) {
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "[ERROR] Failed to load image data.\n");
        DebugPrint(msg);
        return -1;
    }
    if (type == QualityType::FOCUS)
        quality = FocusQuality(srcImage);
    if (type == QualityType::BRIGHTNESS)
        quality = BrightQuality(srcImage);

    sprintf_s(msg, sizeof(msg) - strlen(msg), "[INFO] Quality: %.3f\n", quality);
    return quality;
}
__declspec(dllexport) int AutoAdjust(int minPosition,
                                     int maxPosition,
                                     int step,
                                     CaptureImage captureImage,
                                     int startPosition,
                                     QualityType type,
                                     SearchStrategyType strategy) {
    float quality = -1.0;
    char msg[256] = "";
    isFocusFirstFlag = true;
    isBrightFirstFlag = true;
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "[INFO] Calling AutoAdjust()....\n");
    if (captureImage == NULL) {
        sprintf_s(msg, sizeof(msg) - strlen(msg), "[ERROR] Empty image capture handler. Please check again!\n");
        DebugPrint(msg);
        return -1;
    }
    if (minPosition > maxPosition ||
        (startPosition > 0 && (startPosition < minPosition || startPosition > maxPosition))) {
        sprintf_s(msg, sizeof(msg) - strlen(msg), "[ERROR] Bad focus position. Please check again!\n");
        DebugPrint(msg);
        return -1;
    }
    if (!g_dynamicMem)
        g_dynamicMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
    int optimumPosition = -1;
    sprintf_s(msg,
              sizeof(msg) - strlen(msg),
              "[INFO] Position range: [%d, %d]. Start search poision: %d and each adjuestment step: %d\n",
              minPosition,
              maxPosition,
              startPosition,
              step);

    auto startSearch = std::chrono::high_resolution_clock::now();
    switch (strategy) {
    case SearchStrategyType::BISECTION:
        sprintf_s(msg, sizeof(msg) - strlen(msg), "[INFO] Using Bisection search strategy!\n");
        optimumPosition = BisectionSearch(minPosition, maxPosition, step, captureImage, startPosition, type);
        break;
    case SearchStrategyType::REFOCUS:
        sprintf_s(msg, sizeof(msg) - strlen(msg), "[INFO] Using Smart scan search strategy!\n");
        optimumPosition = RefocusSearch(minPosition, maxPosition, step, captureImage, startPosition, type);
        break;
    }
    auto endSearch = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endSearch - startSearch).count();
    quality = QueryQuality(optimumPosition, captureImage, type);

    sprintf_s(
        msg,
        sizeof(msg) - strlen(msg),
        "[INFO] Duration of searching optimum focus or brightness: %lld ms\tOptimum position: %d\tQuality: %.3f\n",
        duration,
        optimumPosition,
        quality);
    DebugPrint(msg);
    free(g_dynamicMem);
    g_dynamicMem = NULL;
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "[INFO] Calling AutoAdjust()....Done\n");
    isFocusFirstFlag = true;
    isBrightFirstFlag = true;
    return optimumPosition;
}

__declspec(dllexport) int AutoAdjustFocus(int minPosition,
                                          int maxPosition,
                                          int step,
                                          CaptureImage captureImage,
                                          int startPosition) {
    if (startPosition < 0)
        return AutoAdjust(minPosition,
                          maxPosition,
                          step,
                          captureImage,
                          startPosition,
                          QualityType::FOCUS,
                          SearchStrategyType::BISECTION);
    else
        return AutoAdjust(minPosition,
                          maxPosition,
                          step,
                          captureImage,
                          startPosition,
                          QualityType::FOCUS,
                          SearchStrategyType::REFOCUS);
}

__declspec(dllexport) int AutoAdjustLight(int minPosition,
                                          int maxPosition,
                                          int step,
                                          CaptureImage captureImage,
                                          int startPosition) {
    if (startPosition < 0)
        return AutoAdjust(minPosition,
                          maxPosition,
                          step,
                          captureImage,
                          startPosition,
                          QualityType::BRIGHTNESS,
                          SearchStrategyType::BISECTION);
    else
        return AutoAdjust(minPosition,
                          maxPosition,
                          step,
                          captureImage,
                          startPosition,
                          QualityType::BRIGHTNESS,
                          SearchStrategyType::REFOCUS);
}

__declspec(dllexport) float QueryQuality(int position, CaptureImage captureImage, QualityType type) {
    if (!g_dynamicMem)
        g_dynamicMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
    if (captureImage == NULL)
        return -1.0;
    int length = -1;
    auto startSearch = std::chrono::high_resolution_clock::now();
    if (captureImage(g_dynamicMem, length, position) < 0)
        return -1.0;
    auto endSearch = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endSearch - startSearch).count();
    std::string ret = "[INFO] Duration of calling CaptureImage(): " + std::to_string(duration) + "ms\n";
    DebugPrint(ret.c_str());
    std::string imageData = Base64Decoder(g_dynamicMem, length);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    float quality = -1.0;
    if (type == QualityType::FOCUS)
        quality = FocusQuality(imageMat);
    if (type == QualityType::BRIGHTNESS)
        quality = BrightQuality(imageMat);
    auto endCalling = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endCalling - startSearch).count();
    ret = "[INFO] Duration of calling QueryQuality(): " + std::to_string(duration) + "ms\n";
    DebugPrint(ret.c_str());
    return quality;
}

__declspec(dllexport) float FocusQuality(cv::Mat& image) {
    float quality = -1.0;
    // TODO: calculate the image quality based with the current focus setting
    quality = StatSharpnessTenengrad(image);
    std::string ret = "[INFO] Image focus quality: " + std::to_string(quality);
    DebugPrint(ret.c_str());
    return quality;
}

__declspec(dllexport) float BrightQuality(cv::Mat& image) {
    float quality = -1.0;
    // TODO: calculate the image quality based with the current focus setting
    quality = 255 - abs(StatBrightnessRMS(image) - 150);
    std::string ret = "[INFO] Image brightness quality: " + std::to_string(quality);
    DebugPrint(ret.c_str());
    return quality;
}

int RefocusSearch(int begin, int end, int userStep, CaptureImage captureImage, int start, QualityType type) {
    std::ostringstream result;
    int direction = 1;
    int directChangedTimes = 2;
    if (start < 0 || start < begin || start > end)
        start = (begin + end) / 2;

    int pos = start;
    int previous = start;
    while (pos >= begin && pos <= end) {
        if (direction > 0) {
            while ((pos + direction * userStep) <= end &&
                   QueryQuality(pos, captureImage, type) <
                       QueryQuality(pos + direction * userStep, captureImage, type)) {
                result << "Direction: " << direction << "\t Compare: " << pos << " -> " << pos + direction * userStep
                       << std::endl;
                pos += direction * userStep;
            }
        } else {
            while ((pos + direction * userStep) >= begin &&
                   QueryQuality(pos, captureImage, type) <
                       QueryQuality(pos + direction * userStep, captureImage, type)) {
                result << "Direction: " << direction << "\t Compare: " << pos << " -> " << pos + direction * userStep
                       << std::endl;
                pos += direction * userStep;
            }
        }
        if (pos == start) {
            result << "Change direction " << direction << " -> ";
            direction *= -1;
            std::cout << direction << std::endl;
            if (--directChangedTimes == 0)
                break;
            continue;
        }
        break;
    }
    DebugPrint(result.str().c_str());
    return pos;
}

int BisectionSearch(int start, int end, int user_step, CaptureImage captureImage, int startPosition, QualityType type) {
    if (start > end)
        return -1;
    static int step = (end - start) / 10;
    static int round = 1;
    if (user_step <= 0) {
        step = step / round;
        step = step > 0 ? step : 1;
        round *= 2;
    } else {
        step = user_step;
    }

    int mid = start + (end - start) / 2;
    switch (type) {
    case QualityType::BRIGHTNESS:
        if (isBrightFirstFlag) {
            mid = startPosition < 0 ? mid : startPosition;
            isBrightFirstFlag = false;
        }
        break;
    case QualityType::FOCUS:
        if (isFocusFirstFlag) {
            mid = startPosition < 0 ? mid : startPosition;
            isFocusFirstFlag = false;
        }
        break;
    }
    float midQuality = QueryQuality(mid, captureImage, type);
    float frontQuality = QueryQuality(mid - step, captureImage, type);
    float behindQuality = QueryQuality(mid + step, captureImage, type);

    if (midQuality >= frontQuality && midQuality >= behindQuality)
        return mid;
    else if (midQuality < frontQuality)
        return BisectionSearch(start, mid - 1, user_step, captureImage, startPosition, type);
    else
        return BisectionSearch(mid + 1, end, user_step, captureImage, startPosition, type);
}

/**********
 * Functions to calculate focus quality
 */
float StatSharpnessGradient(cv::Mat& image) {
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

float StatSharpnessTenengrad(cv::Mat& img, const int threshold) {
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

float StatSharpnessLaplacian(cv::Mat& img) {
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

float StatSharpnessVariance(cv::Mat& img) {
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
float StatBrightnessMean(cv::Mat& img) {
    float quality = -1.0;
    // TODO: calculate the image quality based on Mean
    if (img.channels() == 3)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    img.convertTo(img, CV_64F);
    quality = cv::mean(img)[0];
    return quality;
}

float StatBrightnessRMS(cv::Mat& img) {
    float quality = -1.0;
    // TODO: calculate the image quality based on RMS
    if (img.channels() == 3)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    img.convertTo(img, CV_64F);
    cv::multiply(img, img, img);
    quality = std::sqrt(cv::mean(img)[0]);
    return quality;
}

float StatBrightnessFormula(cv::Mat& img) {
    float quality = -1.0;
    // TODO: calculate the image quality based on Formula
    if (img.channels() == 1)
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
    img.convertTo(img, CV_64F);
    cv::Scalar rgb = cv::mean(img);
    quality = std::sqrt((0.241 * rgb[2] * rgb[2] + 0.691 * rgb[1] * rgb[1] + 0.068 * rgb[0] * rgb[0]));
    return quality;
}

float StatBrightnessRMSFormula(cv::Mat& img) {
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
