#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "rotateTransform.h"

using namespace std;
using namespace cv;

extern "C"
{
    __declspec(dllexport) int RotateTransform(char *image, int imageSize, double &angle)
    {
        char msg[256] = "";
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling RotateTransform()....\n");
		std::string imageData = Base64Decoder(image, imageSize);
		std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
		cv::Mat srcImage = imdecode(decodedImage, cv::IMREAD_COLOR);
        if (!srcImage.data)
        {
            sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Error: Failed to load image data.\n");
            DebugPrint(msg);
            return -1;
        }

		// TODO: implement the image rotating transform here

        angle = -10.0;
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Rotate angle: %.2f\n", angle);
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling RotateTransform()....Done\n");
        DebugPrint(msg);
        return 0;
    }
}