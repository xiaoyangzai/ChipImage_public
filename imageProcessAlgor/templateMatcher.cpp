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
static char* g_dynamicTargetMem = NULL;
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
    // showShow the offset of the location of matched target image and the matching quality
    return quality;
}

__declspec(dllexport) int GetUniqueTarget(char* image,
                                          int imageSize,
                                          int targetWidth,
                                          int targetHigh,
                                          int& offsetX,
                                          int& offsetY,
                                          char** outputTargetImage,
                                          char** outputMatchedImage,
                                          uint16_t fontSize) {
    int quality = -1;
    LOG("[INFO] Calling GetUniqueTarget()....\n");
    auto begin = std::chrono::high_resolution_clock::now();
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    LOG("[INFO] Image size: %d x %d\n", imageMat.rows, imageMat.cols);

    if (targetWidth >= imageMat.cols || targetHigh >= imageMat.rows) {
        LOG("[ERROR] size of target image is greater than source image. Target image size: (%d x %d) and source image "
            "size: (%d x %d)\n",
            targetWidth,
            targetHigh,
            imageMat.cols,
            imageMat.rows);
        return -2;
    }
    int originalPosX = imageMat.cols / 2 - targetWidth / 2;
    int originalPosY = imageMat.rows / 2 - targetHigh / 2;
    offsetX = -1;
    offsetY = -1;
    LOG("[INFO] Locate of target image: (%d , %d)\n", originalPosX, originalPosY);
    cv::Rect targetROI = cv::Rect(originalPosX, originalPosY, targetWidth, targetHigh);
    cv::Mat targetMat = imageMat(targetROI);
    LOG("[INFO] Target size: %d x %d\n", targetMat.rows, targetMat.cols);
    cv::Mat imageGray, targetGray;
    cv::cvtColor(imageMat, imageGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(targetMat, targetGray, cv::COLOR_BGR2GRAY);
    cv::Mat result;
    cv::matchTemplate(imageGray, targetGray, result, cv::TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
    offsetX = maxLoc.x - originalPosX;
    offsetY = maxLoc.y - originalPosY;
    quality = static_cast<int>(maxVal * 100);
    if (quality < 98 || offsetX > 1 || offsetY > 1) {
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        LOG("[INFO] Invalid target image. offset on X: %d\t offset on Y: %d and Quality is %d\n",
            offsetX,
            offsetY,
            quality);
        LOG("[INFO] During time: %lldms\n", elapsed);
        return -1;
    }
    LOG("[INFO] Valid target image. offset on X: %d\t offset on Y: %d and Quality is %d\n", offsetX, offsetY, quality);
    LOG("[INFO] Checking if the target image is unique.");
    if (IsUniqueTarget(imageMat, targetMat) < 0) {
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        LOG("[INFO] Not unique target image. Please re-select target area.\n");
        LOG("[INFO] During time: %lldms\n", elapsed);
        return -1;
    }

    LOG("[INFO] The target image is unique and will save the target image and source image with matched box.");
    // save target image first
    if (outputTargetImage) {
        // Encoded output image with Base64
        std::vector<uchar> buffer;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 100};
        cv::imencode(".jpg", targetMat, buffer, params);
        char* ptr = reinterpret_cast<char*>(buffer.data());
        std::string outputImageDate = Base64Encoder(ptr, buffer.size());
        if (!g_dynamicTargetMem) {
            g_dynamicTargetMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
        } else {
            memset(g_dynamicTargetMem, 0, MEM_MAX_SIZE * sizeof(char));
        }
        if (outputImageDate.size() >= MEM_MAX_SIZE) {
            LOG("[ERROR] Destation image buffer is not large enough. Maximum buffer size is %d and Current dumped "
                "image size is %d.\n",
                MEM_MAX_SIZE,
                outputImageDate.size());
            return -1;
        }
        // Save the output image into global memory with 15MB space
        strcpy(g_dynamicTargetMem, outputImageDate.c_str());
        *outputTargetImage = g_dynamicTargetMem;
        LOG("[INFO] save target image\n");
    }

    // Draw the box for the searched target image
    cv::rectangle(imageMat, maxLoc, Point(maxLoc.x + targetMat.cols, maxLoc.y + targetMat.rows), Scalar(0, 255, 0), 5);

    if (outputMatchedImage) {
        // showShow the offset of the location of matched target image and the matching quality
        string text = "X=" + to_string(offsetX) + " Y=" + to_string(offsetY) + " Q=" + to_string(quality);
        putText(imageMat, text, Point(20, 80), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), fontSize);
        // Encoded output image with Base64
        std::vector<uchar> buffer;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 100};
        cv::imencode(".jpg", imageMat, buffer, params);
        char* ptr = reinterpret_cast<char*>(buffer.data());
        std::string outputImageDate = Base64Encoder(ptr, buffer.size());
        if (!g_dynamicMem) {
            g_dynamicMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
        } else {
            memset(g_dynamicMem, 0, MEM_MAX_SIZE * sizeof(char));
        }
        if (outputImageDate.size() >= MEM_MAX_SIZE) {
            LOG("[ERROR] Destation image buffer is not large enough. Maximum buffer size is %d and Current dumped "
                "image size is %d.\n",
                MEM_MAX_SIZE,
                outputImageDate.size());
            return -1;
        }

        // Save the output image into global memory with 15MB space
        strcpy(g_dynamicMem, outputImageDate.c_str());
        *outputMatchedImage = g_dynamicMem;
        LOG("[INFO] save matched image\n");
    }

    LOG("[INFO] Matched Position: %d x %d\tQuality: %d\n", maxLoc.x, maxLoc.y, quality);
    LOG("[INFO] Matched offset X: %d, Y: %d\tQuality: %d\n", offsetX, offsetY, quality);
    LOG("[INFO] Calling GetUniqueTarget()....Done\n");
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
                                      char** outputImage,
                                      uint16_t fontSize) {
    int quality = -1;
    LOG("[INFO] Calling MatchTarget()....\n");
    auto begin = std::chrono::high_resolution_clock::now();
    LOG("[INFO] Image length: %d\tTarget length: %d\n", imageSize, targetSize);
    matchedPosX = originalPosX;
    matchedPosY = originalPosY;
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    LOG("[INFO] Image size: %d x %d\n", imageMat.rows, imageMat.cols);
    std::string targetData = Base64Decoder(target, targetSize);
    std::vector<uchar> decodedTarget(targetData.begin(), targetData.end());
    cv::Mat targetMat = imdecode(decodedTarget, cv::IMREAD_COLOR);
    LOG("[INFO] Target size: %d x %d\n", targetMat.rows, targetMat.cols);

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
        LOG("[INFO] offset : %d x %d\tQuality: %d\n", matchedPosX, matchedPosY, quality);
        return quality;
    }
    // showShow the offset of the location of matched target image and the matching quality
    string text = "X=" + to_string(matchedPosX - originalPosX) + " Y=" + to_string(matchedPosY - originalPosY) +
                  " Q=" + to_string(quality);
    putText(imageMat, text, Point(20, 80), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), fontSize);

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
        LOG("[ERROR] Destation image buffer is not large enough. Maximum buffer size is %d and Current dumped "
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

    LOG("[INFO] Matched Position: %d x %d\tQuality: %d\n", matchedPosX, matchedPosY, quality);
    LOG("[INFO] Calling MatchTarget()....Done\n");
    return quality;
}

__declspec(dllexport) int IsUniqueTarget(cv::Mat& imageMat, cv::Mat& targetMat) {
    LOG("[INFO] Calling IsTargetValid()...\n");
    auto begin = std::chrono::high_resolution_clock::now();
    LOG("[INFO] Image size: %d x %d\n", imageMat.rows, imageMat.cols);
    LOG("[INFO] Target size: %d x %d\n", targetMat.rows, targetMat.cols);

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
    LOG("[INFO] Number of available sub-image area is : %llu\n", candidateList.size());
    for (auto&& roi : candidateList) {
        cv::Mat subImg = imageMat(roi);
        int quality = MatchTargetMat(subImg, targetMat);
        LOG("[INFO] Matched quality in sub-image: %d\n", quality);
        if (quality >= 90) {
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            LOG("[INFO] Invalid target image found in sub-image, quality: %d\n", quality);
            LOG("[INFO] During time: %lldms\n", elapsed);
            return -1;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    LOG("[INFO] Found valid target image. During time: %lldms\n", elapsed);
    LOG("[INFO] Calling IsTargetValid()...Done\n");
    return 0;
}
}
