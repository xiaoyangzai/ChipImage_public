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

    __declspec(dllexport) int AutoFocus(int min_focus, int max_focus, int step, CaptureImage captureImage) {
		int quality = -1;
		char msg[256] = "";
		sprintf_s(msg, sizeof(msg) - strlen(msg), "AutoFocus:\nMax: %d, Min: %d, Step: %d\n", min_focus, max_focus, step);
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
		sprintf_s(msg, sizeof(msg) - strlen(msg), "[AutoFocus] Received data length: %d\nQuality: %d\n",length, quality);
		DebugPrint(msg);
        free(g_dynamicMem);
		return 0;
	}
    __declspec(dllexport) int FocusQuality(cv::Mat& image) {
        int quality = -1;
        //TODO: calculate the image quality based with the current focus setting
        quality = StatSharpnessGradient(image);
        std::string ret = "Image focus quality: " + std::to_string(quality);
		DebugPrint(ret.c_str());
        return quality;
    }

    // int StatSharpnessGradient(cv::Mat& image) {
    //     int quality = -1;
    //     //TODO: calculate the image quality based on gradient 
    //     cv::Mat gradient_x, gradient_y;
    //     cv::Sobel(image, gradient_x, CV_64F, 1, 0, 3);
    //     cv::Sobel(image, gradient_y, CV_64F, 0, 1, 3);
    //     cv::Mat gradient_magnitude = cv::Mat(image.size(), CV_64F);
    //     cv::sqrt(gradient_x.mul(gradient_x) + gradient_y.mul(gradient_y), gradient_magnitude);
    //     double score = cv::mean(gradient_magnitude.mul(gradient_magnitude))[0];
    //     quality = (int)score;
    //     return quality;
    // }

    float StatSharpnessTenengrad(cv::Mat& image, const int threshold=500) {
        int quality = -1;
        //TODO: calculate the image quality based on Tenengrad 
        if (img.channels() == 3)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::Mat gx, gy;
        cv::Sobel(img, gx, CV_64F, 1, 0, 3);
        cv::Sobel(img, gy, CV_64F, 0, 1, 3);
        cv::add(gx.mul(gx), gy.mul(gy), gx);
        cv::compare(gx, threshold, gx, cv::CMP_GT);
        gx.convertTo(gx, CV_64F, 1.0/255);
        quality = cv::mean(gx)[0];
        return quality;
    }

    int StatSharpnessLaplacian(cv::Mat& image) {
        float quality = -1;
        //TODO: calculate the image quality based on Laplacian
        if (img.channels() == 3)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::Laplacian(img, img, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(img, mu, sigma);
        quality = sigma[0]*sigma[0];
        return quality;
    }

    int StatSharpnessVariance(cv::Mat& img) {
        float quality = -1;
        //TODO: calculate the image quality based on Variance
        if (img.channels() == 3)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        img.convertTo(img, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(img, mu, sigma);
        quality = sigma[0]*sigma[0];
        return quality;
    }
}
