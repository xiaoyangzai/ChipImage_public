#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <chrono>
#include <future>
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
    LOG(msg, "[INFO] Calling ImageQuality()....\n");
    float quality = -1.0;
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat srcImage = imdecode(decodedImage, cv::IMREAD_COLOR);
    if (!srcImage.data) {
        LOG(msg, "[ERROR] Failed to load image data.\n");
        return -1;
    }
    if (type == QualityType::FOCUS)
        quality = FocusQuality(srcImage);
    if (type == QualityType::BRIGHTNESS)
        quality = BrightQuality(srcImage);

    LOG(msg, "[INFO] Quality: %.3f\n", quality);
    return quality;
}
__declspec(dllexport) int AutoAdjust(int minPosition,
                                     int maxPosition,
                                     int step,
                                     CaptureImage captureImage,
                                     float& optimumQuality,
                                     int startPosition,
                                     QualityType type,
                                     SearchStrategyType strategy,
                                     int timeout,
                                     float refQuality) {
    float quality = -1.0;
    char msg[256] = "";
    isFocusFirstFlag = true;
    isBrightFirstFlag = true;
    LOG(msg, "[INFO] Calling AutoAdjust()....\n");
    LOG(msg, "[INFO] Refer Quality: %.2f....\n", refQuality);
    if (captureImage == NULL) {
        LOG(msg, "[ERROR] Empty image capture handler. Please check again!\n");
        return -1;
    }
    if (minPosition > maxPosition ||
        (startPosition > 0 && (startPosition < minPosition || startPosition > maxPosition))) {
        LOG(msg, "[ERROR] Bad focus position. Please check again!\n");
        return -1;
    }
    if (!g_dynamicMem)
        g_dynamicMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
    int optimumPosition = -1;
    LOG(msg,
        "[INFO] Position range: [%d, %d]. Start search position: %d and each adjustment step: %d\n",
        minPosition,
        maxPosition,
        startPosition,
        step);

    auto startSearch = std::chrono::high_resolution_clock::now();
    switch (strategy) {
    case SearchStrategyType::BISECTION:
        LOG(msg, "[INFO] Using Bisection search strategy!\n");
        optimumPosition = BisectionSearch(minPosition,
                                          maxPosition,
                                          step,
                                          captureImage,
                                          optimumQuality,
                                          startPosition,
                                          type,
                                          timeout,
                                          refQuality);
        break;
    case SearchStrategyType::REFOCUS:
        LOG(msg, "[INFO] Using Smart scan search strategy!\n");
        optimumPosition = RefocusSearch(minPosition,
                                        maxPosition,
                                        step,
                                        captureImage,
                                        optimumQuality,
                                        startPosition,
                                        type,
                                        timeout,
                                        refQuality);
        break;
    }
    auto endSearch = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endSearch - startSearch).count();
    if (optimumPosition > 0) {
        quality = QueryQuality(optimumPosition, captureImage, type);
        LOG(msg,
            "[INFO] Duration of searching optimum focus or brightness: %lld ms\tOptimum position: %d\tQuality: %.3f\n",
            duration,
            optimumPosition,
            quality);
    } else {
        LOG(msg, "[INFO] Execution Timeout!\n");
    }
    free(g_dynamicMem);
    g_dynamicMem = NULL;
    LOG(msg, "[INFO] Calling AutoAdjust()....Done\n");
    isFocusFirstFlag = true;
    isBrightFirstFlag = true;
    return optimumPosition;
}

__declspec(dllexport) int AutoAdjustFocus(int minPosition,
                                          int maxPosition,
                                          int step,
                                          CaptureImage captureImage,
                                          float& optimumQuality,
                                          int startPosition,
                                          int timeout,
                                          float refQuality) {
    SearchStrategyType type;
    if (startPosition < 0)
        type = SearchStrategyType::BISECTION;
    else
        type = SearchStrategyType::REFOCUS;

    return AutoAdjust(minPosition,
                      maxPosition,
                      step,
                      captureImage,
                      optimumQuality,
                      startPosition,
                      QualityType::FOCUS,
                      type,
                      timeout,
                      refQuality);
}

__declspec(dllexport) int AutoAdjustLight(int minPosition,
                                          int maxPosition,
                                          int step,
                                          CaptureImage captureImage,
                                          float& optimumQuality,
                                          int startPosition,
                                          int timeout,
                                          float refQuality) {
    if (startPosition < 0)
        return AutoAdjust(minPosition,
                          maxPosition,
                          step,
                          captureImage,
                          optimumQuality,
                          startPosition,
                          QualityType::BRIGHTNESS,
                          SearchStrategyType::BISECTION,
                          timeout,
                          refQuality);
    else
        return AutoAdjust(minPosition,
                          maxPosition,
                          step,
                          captureImage,
                          optimumQuality,
                          startPosition,
                          QualityType::BRIGHTNESS,
                          SearchStrategyType::REFOCUS,
                          timeout,
                          refQuality);
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
    char msg[256] = "";
    LOG(msg, "[INFO] Duration of calling CaptureImage(): %ld ms\n", duration);
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
    LOG(msg, "[INFO] Duration of calling QueryQuality(): %ld ms\n", duration);
    return quality;
}

__declspec(dllexport) float FocusQuality(cv::Mat& image) {
    float quality = -1.0;
    // TODO: calculate the image quality based with the current focus setting
    quality = StatSharpnessTenengrad(image);
    char msg[256] = "";
    LOG(msg, "[INFO] Image focus quality: %.2f \n", quality);
    return quality;
}

__declspec(dllexport) float BrightQuality(cv::Mat& image) {
    float quality = -1.0;
    // TODO: calculate the image quality based with the current focus setting
    quality = 255 - abs(StatBrightnessRMS(image) - 150);
    char msg[256] = "";
    LOG(msg, "[INFO] Image brightness quality: %.2f\n", quality);
    return quality;
}

int RefocusSearch(int begin,
                  int end,
                  int userStep,
                  CaptureImage captureImage,
                  float& optimumQuality,
                  int start,
                  QualityType type,
                  int timeout,
                  float refQuality) {
    std::ostringstream result;
    int direction = 1;
    int directChangedTimes = 2;
    if (start < 0 || start < begin || start > end)
        start = (begin + end) / 2;

    int pos = start;
    int previous = start;
    auto startSearch = std::chrono::high_resolution_clock::now();
    char msg[256] = "";
    float currentQuality = -1.0;
    while (pos >= begin && pos <= end) {
        auto endSearch = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endSearch - startSearch).count();
        if (direction > 0) {
            while ((pos + direction * userStep) <= end &&
                   (currentQuality = QueryQuality(pos, captureImage, type)) <
                       QueryQuality(pos + direction * userStep, captureImage, type)) {
                LOG(msg, "[INFO] Direction: %d\t Compare: %d -> %d\n", direction, pos, pos + direction * userStep);
                if (refQuality >= 0 && currentQuality >= refQuality)
                    break;
                pos += direction * userStep;
                if (timeout > 0 && elapsed > timeout) {
                    LOG(msg, "[INFO] Execution Timeout\n");
                    return -1;
                } else {
                    LOG(msg, "[INFO] During Time: %ld ms\n", elapsed);
                }
            }
        } else {
            while ((pos + direction * userStep) >= begin &&
                   (currentQuality = QueryQuality(pos, captureImage, type)) <
                       QueryQuality(pos + direction * userStep, captureImage, type)) {
                LOG(msg, "[INFO] Direction: %d\t Compare: %d -> %d\n", direction, pos, pos + direction * userStep);
                if (refQuality >= 0 && currentQuality >= refQuality)
                    break;
                pos += direction * userStep;
                if (timeout > 0 && elapsed > timeout) {
                    LOG(msg, "[INFO] Execution Timeout\n");
                    return -1;
                } else {
                    LOG(msg, "[INFO] During time of RefocusSearch: %ld ms\n", elapsed);
                }
            }
        }
        LOG(msg, "[INFO] Current quality: %.2f\tRefer quality: %.2f\n", currentQuality, refQuality);
        if (refQuality >= 0 && currentQuality >= refQuality)
            break;
        if (pos == start) {
            LOG(msg, "[INFO] Change direction %d ->", direction);
            direction *= -1;
            std::cout << direction << std::endl;
            if (--directChangedTimes == 0)
                break;
            continue;
        }
        break;
    }
    optimumQuality = currentQuality;
    return pos;
}

int BisectionSearch(int start,
                    int end,
                    int user_step,
                    CaptureImage captureImage,
                    float& optimumQuality,
                    int startPosition,
                    QualityType type,
                    int timeout,
                    float refQuality) {
    optimumQuality = -1.0;
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
    float midQuality = -1;
    float frontQuality = -1;
    float behindQuality = -1;
    auto startSearch = std::chrono::high_resolution_clock::now();
    char msg[256] = "";
    do {
        auto endSearch = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endSearch - startSearch).count();
        if (timeout > 0 && elapsed > timeout) {
            LOG(msg, "[INFO] Execution Timeout\n");
            return -1;
        } else {
            LOG(msg, "[INFO] During time of BisectionSearch: %ld ms\n", elapsed);
        }
        midQuality = QueryQuality(mid, captureImage, type);
        if (refQuality >= 0 && midQuality >= refQuality) {
            LOG(msg,
                "[INFO] Current quality[%.2f] is better than passed refer optimum quality[%.2f]:\n",
                midQuality,
                refQuality);
            break;
        }
        frontQuality = QueryQuality(mid - step, captureImage, type);
        behindQuality = QueryQuality(mid + step, captureImage, type);
        if (midQuality >= frontQuality && midQuality >= behindQuality)
            break;
        else if (midQuality < frontQuality) {
            mid = mid - 1;
            // return BisectionSearch(start, mid - 1, user_step, captureImage, startPosition, type);
        } else {
            mid = mid + 1;
            // return BisectionSearch(mid + 1, end, user_step, captureImage, startPosition, type);
        }

    } while (midQuality < frontQuality || midQuality < behindQuality);
    optimumQuality = midQuality;
    return mid;
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
