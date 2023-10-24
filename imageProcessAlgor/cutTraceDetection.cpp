#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "cutTraceDetection.h"

using namespace cv;

extern "C" {
__declspec(dllexport) int CutTraceDetection(char* image,
                                            int imageSize,
                                            double& traceAngle,
                                            int& traceCenterOffset,
                                            int& tranceWidth,
                                            int& maxTraceWidth,
                                            int& maxArea) {
    LOG("Calling CutTraceDetection()....\n");
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);

    if (!imageMat.data) {
        LOG("Error: Failed to load image data.\n");
        return -1;
    }

    // TODO: implement cutting trace validation here
    traceAngle = -1.0;
    traceCenterOffset = -2;
    tranceWidth = 20;
    maxTraceWidth = 23;
    maxArea = 123;
    LOG("Trace angle with baseline: %.2f\n", traceAngle);
    LOG("Trace center offset with baseline: %d\n", traceCenterOffset);
    LOG("Trace width: %d\n", tranceWidth);
    LOG("Maximum Trace width: %d\n", maxTraceWidth);
    LOG("Maximum curved area: %d\n", maxArea);
    LOG("Calling CutTraceDetection()....Done\n");
    return 0;
}
}