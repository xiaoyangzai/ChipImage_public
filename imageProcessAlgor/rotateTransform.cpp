#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include "cutImageAlgr.h"
#include "rotateTransform.h"
#define MEM_MAX_SIZE 1024*1024*15 

using namespace std;
using namespace cv;

static char *g_dynamicMem = NULL;

extern "C"
{
    __declspec(dllexport) int RotateTransform(char *image, int imageSize, double &rotate_angle)
    {
        char msg[256] = "";
        rotate_angle = -1000.0;
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
        Mat gray;
        cvtColor(srcImage, gray, COLOR_BGR2GRAY);

        Mat edges;
        Canny(gray, edges, 50, 150, 3);

        vector<Vec2f> lines;
        HoughLines(edges, lines, 1, CV_PI / 180, 200);

        double max_length = 0;
        for (size_t i = 0; i < lines.size(); i++)
        {
            float rho = lines[i][0], theta = lines[i][1];
            double a = cos(theta), b = sin(theta);
            double x0 = a * rho, y0 = b * rho;
            Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
            Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));

            double length = sqrt(pow(pt2.x - pt1.x, 2) + pow(pt2.y - pt1.y, 2));

            if (length > max_length)
            {
                max_length = length;
                rotate_angle = atan2(pt2.y - pt1.y, pt2.x - pt1.x);
                rotate_angle = rotate_angle * 180 / CV_PI;
            }
        }

        double a = cos(rotate_angle), b = sin(rotate_angle);
        double x0 = a * max_length, y0 = b * max_length;
        Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
        line(srcImage, pt1, pt2, Scalar(0, 255, 255), 20, LINE_AA);
        if (rotate_angle == -1000.0) {
            sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Failed to calculate the rotate_angle\n");
            sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling RotateTransform()....Done\n");
            DebugPrint(msg);
            return -1;
        }
        if (rotate_angle > 45)
        {
            rotate_angle = -90 + rotate_angle;
        }
        else if (rotate_angle < -45)
        {
            rotate_angle = 90 + rotate_angle;
        }
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Rotate rotate_angle: %.2f\n", rotate_angle);

        // Draw the square in the center of the image
        int centerX = srcImage.cols / 2;
        int centerY = srcImage.rows / 2;
        int size = 100;
        int halfSize = size / 2;
        cv::rectangle(srcImage, Point(centerX - halfSize, centerY - halfSize),
                  Point(centerX + halfSize, centerY + halfSize), Scalar(0, 255, 0), 5);
        line(srcImage, Point(centerX - halfSize, centerY), Point(centerX + halfSize, centerY), Scalar(0, 255, 0), 5);
        line(srcImage, Point(centerX, centerY - halfSize), Point(centerX, centerY + halfSize), Scalar(0, 255, 0), 5);
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling RotateTransform()....Done\n");
        DebugPrint(msg);
        return 0;
    }
}