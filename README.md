# ImageProcessModule
This repo implements some functions to process inputting images and provides a DLL file.
# Software requirements
* CMake 3.13 or higher  (install CMake via the powershell CMD)  
`winget install cmake`  
* Microsoft Visual Studio 2022(recommended)
* [OpenCV](https://opencv.org/releases/)
## How to build DLL
1. clone repo to local
2. create build folder
3. Run cmake within this build folder.  
`cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR=/path/to/opencv/OpenCVConfigcmake ..`
4. Build  
  `cd build`  
  `cmake --build . --config Release --verbose`  
  *Note: DLL will be located at build/bin/Release/*  
## How to run unit test
  Place 2 images named `"image.jpg"` and `"target.jpg"` under the folder `build/bin/Release/` before running the test case.  
  Run unit test following below steps.   
  `cd build/bin/Release/`  
  `unit_test.exe`  
# Features
- [x] Provide the debug infomation and callback function to output the debug msg.
- [x] Provide the decoder/encoder fucntion for handle the binary data with the Base64.
- [x] Basic mock functions for the required API  
