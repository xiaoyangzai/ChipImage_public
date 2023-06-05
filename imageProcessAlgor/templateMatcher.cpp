#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "templateMatcher.h"
using namespace cv;
using namespace std;
extern "C"
{
	// Template matcher
	__declspec(dllexport) int MatchTarget(char *image, int imageSize, char *target, int targetSize, int &loc_x, int &loc_y)
	{
		int quality = -1;
		char msg[256] = "";
		sprintf_s(msg, sizeof(msg) - strlen(msg), "Image length: %d\nTarget length: %d\n", imageSize, targetSize);
		loc_x = -1;
		loc_y = -1;
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
		loc_x = maxLoc.x;
		loc_y = maxLoc.y;
		quality = static_cast<int>(maxVal * 100);
		sprintf_s(msg, sizeof(msg) - strlen(msg), "Matched Position: %d x %d\nQuality: %d\n", loc_x, loc_y, quality);
		DebugPrint(msg);
		return quality;
	}
}
