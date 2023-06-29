#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <cutImageAlgr.h>
#include <templateMatcher.h>
#include <rotateTransform.h>
#include <pixelSizeMeter.h>
#include <cutBaseLineDetection.h>
#include <cutTraceDetection.h>
#include <focusQuality.h>
#include <brightQuality.h>
using namespace std;
using namespace cv;

void test_base64_text();
void test_base64_image_show(std::string path);
void test_base64_image_matcher(std::string image, std::string target);
void test_base64_image_rotate_transform(std::string image);
void test_image_pixel_size_measure(std::string image);
void test_cut_baseline_detection(std::string image);
void test_cut_trace_validation(std::string image);
void test_focus_quality_validation(std::string image);
void test_bright_quality_validation(std::string image);
int main(int argc, char *argv[])
{
    int index = -1;
    do
    {
        std::cout << "Function list: \n\t1. Base64 encoder and decoder."
                  << "\n\t2. Show image via encoded/decoded data."
                  << "\n\t3. Image template matcher."
                  << "\n\t4. Image rotating transform."
                  << "\n\t5. Image pixel size measurement."
                  << "\n\t6. Cut baseline detection."
                  << "\n\t7. Cut trace validation."
                  << "\n\t8. Calculate image focus quality."
                  << "\n\t9. Calculate image bright quality."
                  << "\nWhich one you want to test: ";
        std::cin >> index;
    } while (0);

    switch (index)
    {
    case 1:
        test_base64_text();
        break;
    case 2:
        test_base64_image_show("image.jpg");
        break;
    case 3:
        test_base64_image_matcher("image.jpg", "target.jpg");
        break;
    case 4:
        test_base64_image_rotate_transform("rotate_test.jpg");
        break;
    case 5:
        test_image_pixel_size_measure("image.jpg");
        break;
    case 6:
        test_cut_baseline_detection("image.jpg");
        break;
    case 7:
        test_cut_trace_validation("image.jpg");
        break;
    case 8:
        test_focus_quality_validation("image.jpg");
        break;
    case 9:
        test_bright_quality_validation("image.jpg");
        break;
    default:
        break;
    }
    return 0;
}

void test_base64_text()
{
    char msg[] = "hello world";
    std::cout << "msg: " << msg << std::endl;
    auto imageEncodedData = Base64Encoder(msg, strlen(msg));
    std::cout << "Encoded msg: " << imageEncodedData << std::endl;
    auto imageDecodedData = Base64Decoder(&imageEncodedData[0], imageEncodedData.size());
    std::cout << "Decoded msg: " << imageDecodedData << std::endl;
}

void test_base64_image_show(std::string path)
{
    Mat img = imread(path);
    vector<uchar> data;
    std::cout << "start show image...\n";
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());

    string decodedData = Base64Decoder(&encodedImg[0], encodedImg.size());
    vector<uchar> decodedImage(decodedData.begin(), decodedData.end());
    Mat decodedMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    cv::imshow("image", decodedMat);
    cv::waitKey(0);
}

void test_base64_image_matcher(std::string image, std::string target)
{
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());

    data.clear();
    Mat targetMat = imread(target);
    imencode(".jpg", targetMat, data);
    string encodedTarget = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());
    int original_x = 600;
    int original_y = 500;
    int matched_x = 0;
    int matched_y = 0;
    auto quality = MatchTarget(&encodedImg[0], encodedImg.size(), &encodedTarget[0], encodedTarget.size(), original_x, original_y, matched_x, matched_y, NULL);
    if (quality < 0) {
        std::cout << "Error happened during searhing the template image.\n";
        return;
    }
    cv::rectangle(img, cv::Point(matched_x, matched_y), cv::Point(matched_x+ targetMat.cols, matched_y + targetMat.rows), cv::Scalar(0, 255, 0), 2);
    std::ostringstream text;
    text << "Quality: " << quality << " X: " << matched_x - original_x << " Y: " << matched_y - original_y;
    cv::putText(img, text.str(), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 5);

    cv::imshow("Target Image", img);
    cv::waitKey(0);
    std::cout << "Quality: " << quality << " X: " << matched_x - original_x << " Y: " << matched_y - original_y;
}

void test_base64_image_rotate_transform(std::string image)
{
    Mat img = imread(image);
    if (!img.data)
    {
        std::cout << "Failed to load image " << image << "\nError: " << strerror(errno) << std::endl;
        return;
    }
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());
    double angle = 0.0;
    int ret = RotateTransform(&encodedImg[0], encodedImg.size(), angle, NULL);
    std::cout << "Angle: " << angle << std::endl;
    cv::Mat rotate_img;
    cv::Point2f center((1.0 * img.cols) / 2.0, (1.0 * img.rows) / 2.0);
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::warpAffine(img, rotate_img, rotation_matrix, img.size());
    cv::imshow("soure Image", img);
    cv::imshow("rotated Image", rotate_img);
    cv::waitKey(0);
}

void test_image_pixel_size_measure(std::string image)
{
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());
    int pixelSize = 0;
    int ret = PixelSizeMeasure(&encodedImg[0], encodedImg.size(), 8, pixelSize);
    std::cout << "Pixel Size: " << pixelSize << std::endl;
}

void test_cut_baseline_detection(std::string image)
{
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());
    int delta_x = -1;
    int delta_y = -1;
    int ret = CutLineDetection(&encodedImg[0], encodedImg.size(), delta_x, delta_y);
    std::cout << "Delta X: " << delta_x << "\tDelta Y: " << delta_y << std::endl;
}

void test_cut_trace_validation(std::string image)
{
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char *>(data.data()), data.size());

    double traceAngle = 0.0;
    int traceCenterOffset = 0;
    int tranceWidth = -1;
    int maxTraceWidth = -1;
    int maxArea = -1;
    int ret = CutTraceDetection(&encodedImg[0], encodedImg.size(), traceAngle, traceCenterOffset, tranceWidth, maxTraceWidth, maxArea);
}
void test_focus_quality_validation(std::string image) {
    Mat img = imread(image);
    int quality = FocusQuality(img);
    return;
}
void test_bright_quality_validation(std::string image) {
    Mat img = imread(image);
    int quality = BrightQuality(img);
    return;
}