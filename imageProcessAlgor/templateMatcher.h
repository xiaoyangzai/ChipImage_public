
#pragma once

#include <iostream>
extern "C" {
__declspec(dllexport) int MatchTargetMat(cv::Mat& imageMat, cv::Mat& targetMat);
/**
 * @brief Search target from source image and return the location of the matched target image.
 * @param image 		source image data buffer encoded with Base64
 * @param imageSize		buffer size
 * @param target 		target image data buffer encoded with Base64
 * @param targetSize	buffer size
 * @param originalPosX	the X location of target image from the original image
 * @param originalPosY	the Y location of target image from the original image
 * @param matchedPosX	save the X location of matched target image
 * @param matchedPosX	save the Y location of matched target image
 * @param outputImage 	point to the output image buffer
 * @param fontSize      the text font size. defult is 5
 * @return matched quality if successful. otherwise return non zero if failed.
 */
__declspec(dllexport) int MatchTarget(char* image,
                                      int imageSize,
                                      char* target,
                                      int targetSize,
                                      int originalPosX,
                                      int originalPosY,
                                      int& matechedPosX,
                                      int& matchedPosY,
                                      char** outputImage,
                                      uint16_t fontSize = 5);

/**
 * @brief check and return the target image if it is unique withi the souce image.
 * @param image 		source image data buffer encoded with Base64
 * @param imageSize		buffer size
 * @param targetWidth   buffer size
 * @param targetHigh 	high of the selected target image
 * @param offsetX       save the X location offset of matched target image
 * @param offsetY       save the Y location offset of matched target image
 * @param outputTargetImage 	point to the output image buffer saved the unique target image
 * @param outputMatchedImage 	point to the output image buffer with matched infomation
 * @param fontSize      the text font size. defult is 5
 * @return matched quality if successful. otherwise return non zero if failed.
 */
__declspec(dllexport) int GetUniqueTarget(char* image,
                                          int imageSize,
                                          int targetWidth,
                                          int targetHigh,
                                          int& offsetX,
                                          int& offsetY,
                                          char** outputTargetImage,
                                          char** outputMatchedImage,
                                          uint16_t fontSize = 5);

/***
 * @brief check if the provided target template image is valid.
 * @param imageMat 		source image data buffer encoded with cv::Mat
 * @param targetMat 	target image data buffer encoded with cv::Mat
 * @return return 0 if target template image is unique. Otherwise, return -1.
 */
__declspec(dllexport) int IsUniqueTarget(cv::Mat& imageMat, cv::Mat& targetMat);
}