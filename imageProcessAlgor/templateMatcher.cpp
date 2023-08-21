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
static char *g_dynamicMem = NULL;
extern "C"
{
	// Template matcher
	__declspec(dllexport) int MatchTarget(char *image, int imageSize, char *target, int targetSize, int originalPosX, int originalPosY, int &matchedPosX, int &matchedPosY, char **outputImage)
	{
		int quality = -1;
		char msg[256] = "";
		sprintf_s(msg, sizeof(msg) - strlen(msg), "Image length: %d\nTarget length: %d\n", imageSize, targetSize);
		matchedPosX = originalPosX;
		matchedPosY = originalPosY;
		std::string imageData = Base64Decoder(image, imageSize);
		std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
		cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
		sprintf_s(msg, sizeof(msg) - strlen(msg), "Image size: %d x %d\n", imageMat.rows, imageMat.cols);
		std::string targetData = Base64Decoder(target, targetSize);
		std::vector<uchar> decodedTarget(targetData.begin(), targetData.end());
		cv::Mat targetMat = imdecode(decodedTarget, cv::IMREAD_COLOR);
		sprintf_s(msg, sizeof(msg) - strlen(msg), "Target size: %d x %d\n", targetMat.rows, targetMat.cols);

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
		cv::rectangle(imageMat, maxLoc, Point(matchedPosX + targetMat.cols, matchedPosY + targetMat.rows), Scalar(0, 255, 0), 5);

		if (!outputImage)
		{
			sprintf_s(msg, sizeof(msg) - strlen(msg), "offset : %d x %d\nQuality: %d\n", matchedPosX, matchedPosY, quality);
			DebugPrint(msg);
			return quality;
		}
		// showShow the offset of the location of matched target image and the matching quality
		string text = "X=" + to_string(matchedPosX - originalPosX) + " Y=" + to_string(matchedPosY - originalPosY) + " Q=" + to_string(quality);
		putText(imageMat, text, Point(20, 80), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 5);

		// Encoded output image with Base64
		std::vector<uchar> buffer;
		std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 100};
		cv::imencode(".jpg", imageMat, buffer, params);
		char *ptr = reinterpret_cast<char *>(buffer.data());
		std::string outputImageDate = Base64Encoder(ptr, buffer.size());
		if (!g_dynamicMem)
		{
			g_dynamicMem = (char *)malloc(MEM_MAX_SIZE * sizeof(char));
		}
		if (outputImageDate.size() >= MEM_MAX_SIZE)
		{
			sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "[ERROR] Destation image buffer is not large enough. Maximum buffer size is %d and Current dumped image size is %d.\n", MEM_MAX_SIZE, outputImageDate.size());
			return -1;
		}

		// Save the output image into global memory with 15MB space
		strcpy(g_dynamicMem, outputImageDate.c_str());
		if (outputImage)
		{
			*outputImage = g_dynamicMem;
		}

		sprintf_s(msg, sizeof(msg) - strlen(msg), "Matched Position: %d x %d\nQuality: %d\n", matchedPosX, matchedPosY, quality);
		DebugPrint(msg);
		return quality;
	}
}
