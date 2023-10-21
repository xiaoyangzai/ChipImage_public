#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "cutBaseLineDetection.h"
using namespace cv;
using namespace std;
extern "C" {
__declspec(dllexport) int CutLineDetection(char* image, int imageSize, int& delta_x, int& delta_y) {
    char msg[256] = "";
    LOG(msg, "[INFO] Calling CutLineDetection()....\n");
    delta_x = -1;
    delta_y = -1;
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);

    if (!imageMat.data) {
        LOG(msg, "[INFO] Error: Failed to load image data.\n");
        return -1;
    }

    // TODO: implement the cut line detection here

    delta_x = 123;
    delta_y = 222;
    LOG(msg, "[INFO] Delta X: %d\tDelta Y: %d\n", delta_x, delta_y);
    LOG(msg, "[INFO] Calling MatchTarget()....Done\n");
    return 0;
}
}