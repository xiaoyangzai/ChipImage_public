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
        cv::Mat gray;
        cv::cvtColor(srcImage, gray, cv::COLOR_BGR2GRAY);
        cv::Mat edges;
        cv::Canny(gray, edges, 50, 150, 3);

        // 霍夫变换
        std::vector<cv::Vec2f> lines;
        cv::HoughLines(edges, lines, 1, CV_PI / 180, 300);

        double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        if (lines.size() > 0)
        {
            float rho = lines[0][0];
            float theta = lines[0][1];
            double a = cos(theta);
            double b = sin(theta);
            double x0 = a * rho;
            double y0 = b * rho;
            x1 = cvRound(x0 + 1000 * (-b));
            y1 = cvRound(y0 + 1000 * (a));
            x2 = cvRound(x0 - 1000 * (-b));
            y2 = cvRound(y0 - 1000 * (a));
        }

        if (x1 == x2 || y1 == y2)
        {
            std::cerr << "No valid lines found." << std::endl;
            return -1;
        }

        double t = static_cast<double>(y2 - y1) / (x2 - x1);
        double rotate_angle = atan(t) * 180 / CV_PI;
        std::cout << "Rotate angle: " << rotate_angle << std::endl;

        if (rotate_angle > 45)
        {
            rotate_angle = -90 + rotate_angle;
        }
        else if (rotate_angle < -45)
        {
            rotate_angle = 90 + rotate_angle;
        }

        cv::Mat rotate_img;
        cv::Point2f center((1.0 * srcImage.cols) / 2.0, (1.0 * srcImage.rows) / 2.0);
        cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, rotate_angle, 1.0);
        cv::warpAffine(srcImage, rotate_img, rotation_matrix, srcImage.size());

        cv::imshow("Original Image", srcImage);
        cv::imshow("Rotated Image", rotate_img);
        cv::waitKey(0);

        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Rotate angle: %.2f\n", rotate_angle);
        sprintf_s(msg + strlen(msg), sizeof(msg) - strlen(msg), "Calling RotateTransform()....Done\n");
        DebugPrint(msg);
        return 0;
    }
}