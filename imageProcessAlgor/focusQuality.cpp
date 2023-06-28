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
        quality = 78;
        std::string ret = "Image focus quality: " + std::to_string(quality);
		DebugPrint(ret.c_str());
        return quality;
    }
}
