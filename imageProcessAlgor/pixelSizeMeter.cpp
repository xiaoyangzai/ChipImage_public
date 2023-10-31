#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include "cutImageAlgr.h"
#include "templateMatcher.h"
#include "pixelSizeMeter.h"

using namespace cv;

extern "C" {
__declspec(dllexport) int PixelSizeMeasure(char* postImage,
                                           int postImageSize,
                                           char* target,
                                           int targetSize,
                                           int& offsetOnX,
                                           int& offsetOnY,
                                           char** outputImage,
                                           uint16_t fontSize) {
    LOG("[INFO] Calling PixelSizeMeasure()....\n");
    // TODO: implement the pixel size measurement here
    std::string imageData = Base64Decoder(postImage, postImageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    std::string targetData = Base64Decoder(target, targetSize);
    std::vector<uchar> decodedTarget(targetData.begin(), targetData.end());
    cv::Mat targetMat = imdecode(decodedTarget, cv::IMREAD_COLOR);
    int originalPosX = imageMat.cols / 2 - targetMat.cols / 2;
    int originalPosY = imageMat.rows / 2 - targetMat.rows / 2;
    LOG("[INFO] original location of selected target image is : (%d , %d)\n", originalPosX, originalPosY);
    int matchedPosX = -1;
    int matchedPosY = -1;
    int quality =
        MatchTarget(imageMat, targetMat, originalPosX, originalPosY, matchedPosX, matchedPosY, outputImage, fontSize);
    if (quality < 0) {
        return -1;
    }
    offsetOnX = matchedPosX - originalPosX;
    offsetOnY = matchedPosY - originalPosY;
    LOG("[INFO] Quality: %d\toffset on X: %d\toffset on Y: %d\n", quality, offsetOnX, offsetOnY);
    LOG("[INFO] Calling PixelSizeMeasure()....Done\n");
    return quality;
}
}