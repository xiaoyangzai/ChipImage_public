#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "pixelSizeMeter.h"

using namespace cv;

extern "C"
{
    __declspec(dllexport) int PixelSizeMeasure(char *image, int imageSize, int focalDistance, int &pixelSize)
    {
        char msg[256] = "";
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling PixelSizeMeasure()....\n");
        std::string imageData = Base64Decoder(image, imageSize);
        std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
        cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);

        if (!imageMat.data)
        {
            sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Error: Failed to load image data.\n");
            DebugPrint(msg);
            return -1;
        }

        // TODO: implement the pixel size measurement here

        pixelSize = 15;
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Pixel Size: %d\n", pixelSize);
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling PixelSizeMeasure()....Done\n");
        DebugPrint(msg);
        return 0;
    }
}