#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "focusQuality.h"
using namespace cv;
using namespace std;
#define MEM_MAX_SIZE 1024*1024*15 
static char *g_dynamicMem = NULL;
extern "C"
{

    __declspec(dllexport) int AutoBright(int minBright, int maxBright, int step, CaptureImage captureImage) {
		float quality = -1;
		char msg[256] = "";
		sprintf_s(msg, sizeof(msg) - strlen(msg), "AutoBright:\nMax: %d, Min: %d, Step: %d\n", minBright, maxBright, step);
        if (captureImage == NULL) {
		    sprintf_s(msg, sizeof(msg) - strlen(msg), "Empty image capture handler. Please check again!\n");
		    DebugPrint(msg);
            return -1;
        }
        if (!g_dynamicMem) {
            g_dynamicMem = (char*)malloc(MEM_MAX_SIZE * sizeof(char));
        }
        int length = -1;
        if (captureImage(g_dynamicMem, length, 5) < 0) {
		    sprintf_s(msg, sizeof(msg) - strlen(msg), "Invaild image capture handler. Please check again!\n");
		    DebugPrint(msg);
            return -1;
        }
		std::string imageData = Base64Decoder(g_dynamicMem, length);
		std::vector<uchar> decodedImage(imageData.begin(), imageData.end());
		cv::Mat imageMat = imdecode(decodedImage, cv::IMREAD_COLOR);
        quality = FocusQuality(imageMat);
		sprintf_s(msg, sizeof(msg) - strlen(msg), "[AutoBright] Received data length: %d\nQuality: %d\n",length, quality);
		DebugPrint(msg);
        free(g_dynamicMem);
		return 0;
	}

    __declspec(dllexport) float BrightQuality(cv::Mat& image) {
        float quality = -1;
        //TODO: calculate the image quality based with the current focus setting
        quality = StatBrightnessRMS(image);
        std::string ret = "Image focus quality: " + std::to_string(quality);
		DebugPrint(ret.c_str());
        return quality;
    }

    float StatBrightnessMean(cv::Mat& image) {
        float quality = -1;
        //TODO: calculate the image quality based on Mean
        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
            img.convertTo(img, CV_64F);
        quality = cv::mean(img)[0];
        return quality;
    }

    float StatBrightnessRMS(cv::Mat& image) {
        float quality = -1;
        //TODO: calculate the image quality based on RMS 
        if (img.channels() == 3)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::multiply(img, img, img);
        quality = std::sqrt(cv::mean(img)[0]);
        return quality;
    }

    float StatBrightnessFormula(cv::Mat& image) {
        int quality = -1;
        //TODO: calculate the image quality based on Formula 
        if (img.channels() == 1)
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
        img.convertTo(img, CV_64F);
        cv::Scalar rgb = cv::mean(img);
        quality = std::sqrt((0.241 * rgb[2]*rgb[2] + 0.691 * rgb[1]*rgb[1] + 0.068 * rgb[0]*rgb[0]));
        return quality;
    }

    float StatBrightnessRMSFormula(cv::Mat& image) {
        int quality = -1;
        //TODO: calculate the image quality based on RMS Formula 
        if (img.channels() == 1)
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
        img.convertTo(img, CV_64F);
        cv::multiply(img, img, img);
        cv::Scalar rms = cv::mean(img);
        quality = std::sqrt((0.241 * rms[2] + 0.691 * rms[1] + 0.068 * rms[0]));
        return quality;
    }
}
