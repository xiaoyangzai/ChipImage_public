
#pragma once

#include <iostream>
extern "C"
{
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
	 * @return matched quality if successful. otherwise return non zero if failed.
	 */
	__declspec(dllexport) int MatchTarget(char *image, int imageSize, char *target, int targetSize, int originalPosX, int originalPosY, int &matechedPosX, int &matchedPosY, char **outputImage);
}