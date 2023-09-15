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

extern "C"
{
	__declspec(dllexport) int PixelSizeMeasure(char *postImage, int postImageSize, char *target, int targetSize, int& offsetOnX, int& offsetOnY, char **outputImage)
    {
        char msg[256] = "";
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling PixelSizeMeasure()....\n");
        // TODO: implement the pixel size measurement here
		std::string imageData = Base64Decoder(postImage, postImageSize);
		std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
		cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);

		std::string targetData = Base64Decoder(target, targetSize);
		std::vector<uchar> decodedTarget(targetData.begin(), targetData.end());
		cv::Mat targetMat = imdecode(decodedTarget, cv::IMREAD_COLOR);
        int matchedPosX = -1;
        int matchedPosY = -1;
        int rand_x = rand()%5;
        int rand_y = rand()%5;
        int quality = MatchTarget(postImage, postImageSize, target, targetSize, imageMat.rows / 2 - targetMat.rows / 2 + rand_x, imageMat.cols / 2 - targetMat.cols / 2 + rand_y, matchedPosX, matchedPosY, outputImage);
        if (quality < 0) {
            return -1;
        }
        offsetOnX = matchedPosX + targetMat.cols / 2 - imageMat.cols / 2 - rand_x;
        offsetOnY = matchedPosY + targetMat.rows / 2 - imageMat.rows / 2 - rand_y;
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Quality: %d\toffset on X: %d\toffset on Y: %d\n", quality, offsetOnX, offsetOnY);
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling PixelSizeMeasure()....Done\n");
        DebugPrint(msg);
        return quality;
    }
}