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
#include <autoAdjustQuality.h>
using namespace std;
using namespace cv;

void test_base64_text();
void test_base64_image_show(std::string path);
void test_base64_image_matcher(std::string image, std::string target);
void test_base64_image_unique_target(std::string image);
void test_unique_target_image(std::string image, std::string target);
void test_base64_image_rotate_transform(std::string image);
void test_image_pixel_size_measure(std::string image, std::string target);
void test_cut_baseline_detection(std::string image);
void test_cut_trace_validation(std::string image);
void test_focus_quality_validation(std::string image);
void test_bright_quality_validation(std::string image);
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage: ./unit_test.exe /path/to/image.jpg [/path/to/target.jpg]\n";
        return -1;
    }
    int index = -1;
    do {
        std::cout << "Function list: \n\t1. Base64 encoder and decoder."
                  << "\n\t2. Show image via encoded/decoded data."
                  << "\n\t3. Image template matcher."
                  << "\n\t4. Image rotating transform."
                  << "\n\t5. Image pixel size measurement."
                  << "\n\t6. Cut baseline detection."
                  << "\n\t7. Cut trace validation."
                  << "\n\t8. Calculate image focus quality."
                  << "\n\t9. Calculate image bright quality."
                  << "\n\t10. Check if target image is unique via selected target image."
                  << "\n\t11. Check if target image is unique via size of selected targe timage."
                  << "\nWhich one you want to test: ";
        std::cin >> index;
    } while (0);

    switch (index) {
    case 1:
        test_base64_text();
        break;
    case 2:
        test_base64_image_show(argv[1]);
        break;
    case 3:
        test_base64_image_matcher(argv[1], argv[2]);
        break;
    case 4:
        test_base64_image_rotate_transform(argv[1]);
        break;
    case 5:
        test_image_pixel_size_measure(argv[1], argv[2]);
        break;
    case 6:
        test_cut_baseline_detection(argv[1]);
        break;
    case 7:
        test_cut_trace_validation(argv[1]);
        break;
    case 8:
        test_focus_quality_validation(argv[1]);
        break;
    case 9:
        test_bright_quality_validation(argv[1]);
        break;
    case 10:
        test_unique_target_image(argv[1], argv[2]);
        break;
    case 11:
        test_base64_image_unique_target(argv[1]);
        break;
    default:
        break;
    }
    return 0;
}

void test_base64_text() {
    char msg[] = "hello world";
    std::cout << "msg: " << msg << std::endl;
    auto imageEncodedData = Base64Encoder(msg, strlen(msg));
    std::cout << "Encoded msg: " << imageEncodedData << std::endl;
    auto imageDecodedData = Base64Decoder(&imageEncodedData[0], imageEncodedData.size());
    std::cout << "Decoded msg: " << imageDecodedData << std::endl;
}

void test_base64_image_show(std::string path) {
    Mat img = imread(path);
    vector<uchar> data;
    std::cout << "start show image...\n";
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());

    string decodedData = Base64Decoder(&encodedImg[0], encodedImg.size());
    vector<uchar> decodedImage(decodedData.begin(), decodedData.end());
    Mat decodedMat = imdecode(decodedImage, cv::IMREAD_COLOR);
    cv::imshow("image", decodedMat);
    cv::waitKey(0);
}

void test_base64_image_unique_target(std::string image) {
    char* matchedImage = NULL;
    char* targetImage = NULL;
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());
    int offsetX = -1;
    int offsetY = -1;
    int wide = img.cols / 3;
    int high = img.rows / 3;
    auto quality =
        GetUniqueTarget(&encodedImg[0], encodedImg.size(), wide, high, offsetX, offsetY, &targetImage, &matchedImage);

    if (quality < 0) {
        std::cout << "not unique target image.\n";
        return;
    }
    if (matchedImage) {
        string encodedImg(matchedImage);
        string decodedData = Base64Decoder(&encodedImg[0], encodedImg.size());
        vector<uchar> decodedImage(decodedData.begin(), decodedData.end());
        Mat decodedMat = imdecode(decodedImage, cv::IMREAD_COLOR);
        cv::imshow("Matched image", decodedMat);
        cv::waitKey(0);
    }
    if (targetImage) {
        string encodedImg(targetImage);
        string decodedData = Base64Decoder(&encodedImg[0], encodedImg.size());
        vector<uchar> decodedImage(decodedData.begin(), decodedData.end());
        Mat decodedMat = imdecode(decodedImage, cv::IMREAD_COLOR);
        cv::imshow("target image", decodedMat);
        cv::waitKey(0);
    }
    std::cout << "Quality: " << quality << " offset X: " << offsetX << " offset Y: " << offsetY << std::endl;
}

void test_base64_image_matcher(std::string image, std::string target) {
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());

    data.clear();
    Mat targetMat = imread(target);
    imencode(".jpg", targetMat, data);
    string encodedTarget = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());
    int original_x = 700;
    int original_y = 450;
    int matched_x = 0;
    int matched_y = 0;
    auto quality = MatchTarget(&encodedImg[0],
                               encodedImg.size(),
                               &encodedTarget[0],
                               encodedTarget.size(),
                               original_x,
                               original_y,
                               matched_x,
                               matched_y,
                               NULL);
    if (quality < 0) {
        std::cout << "Error happened during searhing the template image.\n";
        return;
    }
    cv::rectangle(img,
                  cv::Point(matched_x, matched_y),
                  cv::Point(matched_x + targetMat.cols, matched_y + targetMat.rows),
                  cv::Scalar(0, 255, 0),
                  2);
    std::ostringstream text;
    text << "Quality: " << quality << " X: " << matched_x - original_x << " Y: " << matched_y - original_y;
    cv::putText(img, text.str(), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 5);

    cv::imshow("Target Image", img);
    cv::waitKey(0);
    std::cout << "Quality: " << quality << " X: " << matched_x - original_x << " Y: " << matched_y - original_y;
}

void test_unique_target_image(std::string image, std::string target) {
    Mat imgMat = imread(image);
    Mat targetMat = imread(target);
    int ret = IsUniqueTarget(imgMat, targetMat);
    if (ret < 0) {
        std::cout << "Invalid target image. Please re-select target image.\n";
        return;
    }
    std::cout << "Found unique target image: " << std::endl;
}

void test_base64_image_rotate_transform(std::string image) {
    Mat img = imread(image);
    if (!img.data) {
        std::cout << "Failed to load image " << image << "\nError: " << strerror(errno) << std::endl;
        return;
    }
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());
    double angle = 0.0;
    int ret = RotateTransform(&encodedImg[0], encodedImg.size(), angle);
    std::cout << "Angle: " << angle << std::endl;
    cv::Mat rotate_img;
    cv::Point2f center((1.0 * img.cols) / 2.0, (1.0 * img.rows) / 2.0);
    cv::Mat rotation_matrix = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::warpAffine(img, rotate_img, rotation_matrix, img.size());
    cv::imshow("soure Image", img);
    cv::imshow("rotated Image", rotate_img);
    cv::waitKey(0);
}

void test_image_pixel_size_measure(std::string image, std::string target) {
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());

    Mat targetMat = imread(target);
    vector<uchar> targetData;
    imencode(".jpg", targetMat, targetData);
    string encodedTarget = Base64Encoder(reinterpret_cast<char*>(targetData.data()), targetData.size());

    int offsetOnX = 0;
    int offsetOnY = 0;
    int quality = PixelSizeMeasure(&encodedImg[0],
                                   encodedImg.size(),
                                   &encodedTarget[0],
                                   encodedTarget.size(),
                                   offsetOnX,
                                   offsetOnY,
                                   NULL);
    std::cout << "Quality: " << quality << std::endl;
    std::cout << "offset on X: " << offsetOnX << "\t offset on Y: " << offsetOnY << std::endl;
}

void test_cut_baseline_detection(std::string image) {
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());
    int delta_x = -1;
    int delta_y = -1;
    int ret = CutLineDetection(&encodedImg[0], encodedImg.size(), delta_x, delta_y);
    std::cout << "Delta X: " << delta_x << "\tDelta Y: " << delta_y << std::endl;
}

void test_cut_trace_validation(std::string image) {
    Mat img = imread(image);
    vector<uchar> data;
    imencode(".jpg", img, data);
    string encodedImg = Base64Encoder(reinterpret_cast<char*>(data.data()), data.size());

    double traceAngle = 0.0;
    int traceCenterOffset = 0;
    int tranceWidth = -1;
    int maxTraceWidth = -1;
    int maxArea = -1;
    int ret = CutTraceDetection(&encodedImg[0],
                                encodedImg.size(),
                                traceAngle,
                                traceCenterOffset,
                                tranceWidth,
                                maxTraceWidth,
                                maxArea);
}
void test_focus_quality_validation(std::string image) {
    Mat img = imread(image);
    float quality = FocusQuality(img);
    std::cout << "Focus Quality: " << quality << std::endl;
    return;
}
void test_bright_quality_validation(std::string image) {
    Mat img = imread(image);
    float quality = BrightQuality(img);
    std::cout << "Brightness Quality: " << quality << std::endl;
    return;
}