#pragma once

#include <iostream>
extern "C" {
/**
 * @brief measure the pixel size
 * @param postImage		the image data buffer after moving encoded with Base64
 * @param postImageSize	buffer size of the image after moving
 * @param templateImage	the template image buffer
 * @param tempImageSize the template image buffer size
 * @param offsetOnX		offset on X dim compared to template image X before moving
 * @param offsetOnY		offset on Y dim compared to template image Y before moving
 * @param outputImage 	point to the output image buffer
 * @param fontSize      the text font size. Default is 5
 * @return 				matched quality if successful, otherwise non-zero will return.
 */
__declspec(dllexport) int PixelSizeMeasure(char* postImage,
                                           int postImageSize,
                                           char* templateImage,
                                           int tempImageSize,
                                           int& offsetOnX,
                                           int& offsetOnY,
                                           char** outputImage,
                                           uint16_t fontSize = 5);
}