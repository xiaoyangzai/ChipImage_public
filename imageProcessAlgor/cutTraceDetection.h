#pragma once

#include <iostream>
extern "C" {
/**
 * @brief check if the cut trace meets the requirement, including
 *          1. angle between the cut line and cut trace.
 *          2. the offset between the cut trace center and the cut line center
 *          3. check the if curved area exceeds the threshold
 * @param image 		            source image data buffer encoded with Base64
 * @param imageSize		            buffer size
 * @param angle                     save angle between the cut tarce and the cut line
 * @param tranceCenterOffset 		the offset between the cut trace center and the cut line center
 * @param tranceWidth               the width of the cut trace
 * @param maxTraceWidth             the maximum width of the cut trace including the curved area
 * @param maxArea                   the maximum pixel number inside the curved area
 * @return return 0 if successful, otherwise non-zero will return.
 */
__declspec(dllexport) int CutTraceDetection(char* image,
                                            int imageSize,
                                            double& angle,
                                            int& traceCenterOffset,
                                            int& tranceWidth,
                                            int& maxTraceWidth,
                                            int& maxArea);
}