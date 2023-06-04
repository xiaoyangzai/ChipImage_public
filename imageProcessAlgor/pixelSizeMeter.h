#pragma once

#include <iostream>
extern "C"
{
	/**
	 * @brief measure the pixel size
	 * @param image 		source image data buffer encoded with Base64
	 * @param imageSize		buffer size
	 * @param focalDistance	the focal distance using for this image
	 * @param pixelSize		save the measured pixel size
	 * @return return 0 if successful, otherwise non-zero will return.
	 */
	__declspec(dllexport) int PixelSizeMeasure(char *image, int imageSize, int focalDistance, int &pixelSize);
}