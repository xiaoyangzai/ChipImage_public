
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
	 * @param loc_x			save the X location of matched target image
	 * @param loc_y			save the Y location of matched target image
	 * @return matched quality if successful. otherwise return non zero if failed.
	 */
	__declspec(dllexport) int MatchTarget(char *image, int imageSize, char *target, int targetSize, int &loc_x, int &loc_y);
}