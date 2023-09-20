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
    char msg[256] = "";
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling CutTraceDetection()....\n");
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);

    if (!imageMat.data) {
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Error: Failed to load image data.\n");
        DebugPrint(msg);
        return -1;
    }

    // TODO: implement cutting trace validation here
    traceAngle = -1.0;
    traceCenterOffset = -2;
    tranceWidth = 20;
    maxTraceWidth = 23;
    maxArea = 123;
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Trace angle with baseline: %.2f\n", traceAngle);
    sprintf_s(msg + strlen(msg),
              sizeof(msg) - strlen(msg),
              "Trace center offset with baseline: %d\n",
              traceCenterOffset);
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Trace width: %d\n", tranceWidth);
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Maximum Trace width: %d\n", maxTraceWidth);
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Maximum curved area: %d\n", maxArea);
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling CutTraceDetection()....Done\n");
    DebugPrint(msg);
    return 0;
}
}