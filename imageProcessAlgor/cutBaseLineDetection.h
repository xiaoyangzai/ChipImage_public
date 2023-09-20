#pragma once

#include <iostream>
extern "C" {
/**
 * @brief measure offset between the center of the image to the center of cut line
 * @param image 		source image data buffer encoded with Base64
 * @param imageSize		buffer size
 * @param delta_x 		save the X offset to cut line
 * @param delta_y 		save the Y offset to cut line
 * @return return 0 if successful, otherwise non-zero will return.
 */
__declspec(dllexport) int CutLineDetection(char* image, int imageSize, int& delta_x, int& delta_y);
}