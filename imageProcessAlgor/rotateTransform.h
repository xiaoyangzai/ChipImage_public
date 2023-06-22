#pragma once

#include <iostream>
extern "C"
{
	/**
	 * @brief calculate the angel of the image
	 * @param image 		source image data buffer encoded with Base64
	 * @param imageSize		buffer size
	 * @param angle 		save the angle
	 * @param outputImage 	point to the output image buffer
	 * @return return 0 if successful, otherwise non-zero will return.
	 */
	__declspec(dllexport) int RotateTransform(char *image, int imageSize, double &angle, char** outputImage);
}