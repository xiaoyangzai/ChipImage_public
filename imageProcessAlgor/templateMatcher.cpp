#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "templateMatcher.h"
using namespace cv;
using namespace std;
#define MEM_MAX_SIZE 1024 * 1024 * 15
static char* g_dynamicMem = NULL;
extern "C" {

__declspec(dllexport) int MatchTargetMat(cv::Mat& imageMat, cv::Mat& targetMat) {
    cv::Mat imageGray, targetGray;
    cv::cvtColor(imageMat, imageGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(targetMat, targetGray, cv::COLOR_BGR2GRAY);
    cv::Mat result;
    cv::matchTemplate(imageGray, targetGray, result, cv::TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
    auto matchedPosX = maxLoc.x;
    auto matchedPosY = maxLoc.y;
    auto quality = static_cast<int>(maxVal * 100);

    // Draw the box for the searched target image
    cv::rectangle(imageMat,
                  maxLoc,
                  Point(matchedPosX + targetMat.cols, matchedPosY + targetMat.rows),
                  Scalar(0, 255, 0),
                  5);
    // showShow the offset of the location of matched target image and the matching quality
    string text = "Q=" + to_string(quality);
    putText(imageMat, text, Point(20, 80), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 5);
    return quality;
}
// Template matcher
__declspec(dllexport) int MatchTarget(char* image,
                                      int imageSize,
                                      char* target,
                                      int targetSize,
                                      int originalPosX,
                                      int originalPosY,
                                      int& matchedPosX,
                                      int& matchedPosY,
                                      char** outputImage) {
    int quality = -1;
    char msg[256] = "";
    LOG(msg, "[INFO] Calling MatchTarget()....\n");
    auto begin = std::chrono::high_resolution_clock::now();
    LOG(msg, "[INFO] Image length: %d\tTarget length: %d\n", imageSize, targetSize);
    matchedPosX = originalPosX;
    matchedPosY = originalPosY;
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    LOG(msg, "[INFO] Image size: %d x %d\n", imageMat.rows, imageMat.cols);
    std::string targetData = Base64Decoder(target, targetSize);
    std::vector<uchar> decodedTarget(targetData.begin(), targetData.end());
    cv::Mat targetMat = imdecode(decodedTarget, cv::IMREAD_COLOR);
    LOG(msg, "[INFO] Target size: %d x %d\n", targetMat.rows, targetMat.cols);

    if (IsUniqueTarget(imageMat, targetMat) < 0) {
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        LOG(msg, "[INFO] Invalid target image\n");
        LOG(msg, "[INFO] During time: %lldms\n", elapsed);
        return -1;
    }

    cv::Mat imageGray, targetGray;
    cv::cvtColor(imageMat, imageGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(targetMat, targetGray, cv::COLOR_BGR2GRAY);
    cv::Mat result;
    cv::matchTemplate(imageGray, targetGray, result, cv::TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
    matchedPosX = maxLoc.x;
    matchedPosY = maxLoc.y;
    quality = static_cast<int>(maxVal * 100);

    // Draw the box for the searched target image
    cv::rectangle(imageMat,
                  maxLoc,
                  Point(matchedPosX + targetMat.cols, matchedPosY + targetMat.rows),
                  Scalar(0, 255, 0),
                  5);

    if (!outputImage) {
        LOG(msg, "[INFO] offset : %d x %d\tQuality: %d\n", matchedPosX, matchedPosY, quality);
        return quality;
    }
    // showShow the offset of the location of matched target image and the matching quality
    string text = "X=" + to_string(matchedPosX - originalPosX) + " Y=" + to_string(matchedPosY - originalPosY) +
                  " Q=" + to_string(quality);
    putText(imageMat, text, Point(20, 80), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 5);

    // Encoded output image with Base64
    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 100};
    cv::imencode(".jpg", imageMat, buffer, params);
    char* ptr = reinterpret_cast<char*>(buffer.data());
    std::string outputImageDate = Base64Encoder(ptr, buffer.size());
    if (!g_dynamicMem) {
        g_dynamicMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
    }
    if (outputImageDate.size() >= MEM_MAX_SIZE) {
        LOG(msg,
            "[ERROR] Destation image buffer is not large enough. Maximum buffer size is %d and Current dumped "
            "image size is %d.\n",
            MEM_MAX_SIZE,
            outputImageDate.size());
        return -1;
    }

    // Save the output image into global memory with 15MB space
    strcpy(g_dynamicMem, outputImageDate.c_str());
    if (outputImage) {
        *outputImage = g_dynamicMem;
    }

    LOG(msg, "[INFO] Matched Position: %d x %d\tQuality: %d\n", matchedPosX, matchedPosY, quality);
    LOG(msg, "[INFO] Calling MatchTarget()....Done\n");
    return quality;
}

__declspec(dllexport) int IsUniqueTarget(cv::Mat& imageMat, cv::Mat& targetMat) {
    char msg[256] = "";
    LOG(msg, "[INFO] Calling IsTargetValid()...\n");
    auto begin = std::chrono::high_resolution_clock::now();
    LOG(msg, "[INFO] Image size: %d x %d\n", imageMat.rows, imageMat.cols);
    LOG(msg, "[INFO] Target size: %d x %d\n", targetMat.rows, targetMat.cols);

    int MarginY = imageMat.cols / 2 - targetMat.cols / 2;
    int MarginX = imageMat.rows / 2 - targetMat.rows / 2;
    std::vector<cv::Rect> candidateList;
    if (MarginX >= targetMat.cols) {
        candidateList.push_back(cv::Rect(0, 0, imageMat.cols, imageMat.rows / 2));
        candidateList.push_back(cv::Rect(0, imageMat.rows / 2, imageMat.cols, imageMat.rows / 2));
    }
    if (MarginY >= targetMat.rows) {
        candidateList.push_back(cv::Rect(0, 0, imageMat.cols / 2, imageMat.rows));
        candidateList.push_back(cv::Rect(imageMat.cols / 2, 0, imageMat.cols / 2, imageMat.rows));
    }
    LOG(msg, "[INFO] Number of available sub-image area is : %llu\n", candidateList.size());
    for (auto&& roi : candidateList) {
        cv::Mat subImg = imageMat(roi);
        int quality = MatchTargetMat(subImg, targetMat);
        LOG(msg, "[INFO] Matched quality in sub-image: %d\n", quality);
        if (quality >= 90) {
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            LOG(msg, "[INFO] Invalid target image found in sub-image, quality: %d\n", quality);
            LOG(msg, "[INFO] During time: %lldms\n", elapsed);
            return -1;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    LOG(msg, "[INFO] Found valid target image. During time: %lldms\n", elapsed);
    LOG(msg, "[INFO] Calling IsTargetValid()...Done\n");
    return 0;
}
}
