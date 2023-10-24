// cutImageAlgr.cpp : Defines the entry point for the application.
//
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "cutImageAlgr.h"
using namespace cv;
using namespace std;
std::ofstream logFile;
std::string g_logFileName = "image_process.log";
extern "C" {
typedef void (*DebugCallback)(const char*);

// Define the callback function pointer
static DebugCallback s_DebugCallback = nullptr;

// Set callback functon
__declspec(dllexport) void SetDebugCallback(DebugCallback callback) {
    s_DebugCallback = callback;
}

// Print the debug infomation like the argument of function calling and returned value.
__declspec(dllexport) void DebugPrint(const char* message) {
    if (s_DebugCallback) {
        s_DebugCallback(message);
    }
    std::cout << message;
    logFile << message;
    logFile.flush();
}

// Basic function test. Just check if the debug informate output works well.
__declspec(dllexport) void BaseFunctionTest(char* data, int length) {
    LOG("Function Name:\n\tBaseFunctionTest\nBrief:\n\tPrint the debug msg to check if Debug callback works\n"
        "Parameters:\n\tdata, char*, Pointer. point to the data buffer;\n\tlength, int, length of the data;\n"
        "Return:\n\tvoid\n Argument:\n\tdata = %s\n\tlength = %d\n",
        data,
        length);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        logFile.open(g_logFileName, std::fstream::out | std::fstream::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file!" << std::endl;
            return FALSE;
        }
        LOG("Loading image_process.dll\n");
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        LOG("Unloading image_process.dll\n");
        if (logFile.is_open())
            logFile.close();
        break;
    }

    return TRUE;
}
}

__declspec(dllexport) std::string Base64Encoder(char* data, int length) {
    std::string source(data, length);
    DWORD binarySize = static_cast<DWORD>(length);
    LPBYTE pbBinary = reinterpret_cast<LPBYTE>(data);

    DWORD dwFlags = CRYPT_STRING_BASE64;
    DWORD stringSize = 0;

    CryptBinaryToString(pbBinary, binarySize, dwFlags, nullptr, &stringSize);

    std::string encodedData(stringSize, '\0');

    if (!CryptBinaryToString(pbBinary, binarySize, dwFlags, const_cast<LPTSTR>(encodedData.data()), &stringSize)) {
        LOG("Function Name:\n\tBase64Encoder\nBrief:\n\tEncode the input string with Base64.\n"
            "Parameters:\n\tdata, char*, Pointer. point to the source data;\n\tlength, int, length of the data;\n"
            "Return:\n\tvoid\n Argument:\n\tdata = %p\n\tlength = %d\n",
            data,
            length);
        LOG("Encoded result: Failed to decode Base64 data. \nError code: %ld\n", GetLastError());
    }
    return encodedData;
}

__declspec(dllexport) std::string Base64Decoder(char* data, int length) {
    // Base64 encoded data
    std::string source(data, length);
    DWORD dwFlags = CRYPT_STRING_BASE64;
    DWORD binarySize = 0;

    CryptStringToBinary(source.c_str(), 0, dwFlags, nullptr, &binarySize, nullptr, nullptr);

    std::string binaryData(binarySize, '\0');

    if (!CryptStringToBinary(source.c_str(),
                             0,
                             dwFlags,
                             reinterpret_cast<LPBYTE>(&binaryData[0]),
                             &binarySize,
                             nullptr,
                             nullptr)) {
        LOG("Function Name:\n\tBase64Decoder\nBrief:\n\tDecode the input string with Base64.\n"
            "Parameters:\n\tdata, char*, Pointer. point to the encoded data;\n\tlength, int, length of the data;\n"
            "Return:\n\tvoid\n Argument:\n\tdata = %p\n\tlength = %d\n",
            data,
            length);
        LOG("Decoded result: Failed to decode Base64 data. \nError code: %ld\n", GetLastError());
    }
    return binaryData;
}