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
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling CutLineDetection()....\n");
    delta_x = -1;
    delta_y = -1;
    std::string imageData = Base64Decoder(image, imageSize);
    std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
    cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);

    if (!imageMat.data) {
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Error: Failed to load image data.\n");
        DebugPrint(msg);
        return -1;
    }

    // TODO: implement the cut line detection here

    delta_x = 123;
    delta_y = 222;
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Delta X: %d\tDelta Y: %d\n", delta_x, delta_y);
    sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling MatchTarget()....Done\n");
    DebugPrint(msg);
    return 0;
}
}