// cutImageAlgr.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#define LOG(msgbuffer, log, ...)                                                                           \
    do {                                                                                                   \
        char timeStr[100] = "";                                                                            \
        time_t t = std::time(nullptr);                                                                     \
        struct tm localTime;                                                                               \
        localtime_s(&localTime, &t);                                                                       \
        std::strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S] ", &localTime);                       \
        sprintf_s(msgbuffer, sizeof(msgbuffer) - strlen(msgbuffer), "%s", timeStr);                        \
        sprintf_s(msgbuffer + strlen(msgbuffer), sizeof(msgbuffer) - strlen(msgbuffer), log, __VA_ARGS__); \
        DebugPrint(msgbuffer);                                                                             \
    } while (0)

extern "C" {
__declspec(dllexport) void BaseFunctionTest(char* data, int length);
/**
 * @brief output the debug message via std::cout or callback function user setting
 * @param message 		debug message string buffer
 * @return return 0 if successful, otherwise non-zero will return.
 */
__declspec(dllexport) void DebugPrint(const char* message);
}

/**
 * @brief encoding the data with the Base64
 * @param data 				the data buffer to encode
 * @param length			the data length of the buffer
 * @return return the encoded string. Empty string will return if failed.
 */
__declspec(dllexport) std::string Base64Encoder(char* data, int length);

/**
 * @brief decoding the data with the Base64
 * @param data 				the data buffer to encode
 * @param length			the data length of the buffer
 * @return return the decoded string. Empty string will return if failed.
 */
__declspec(dllexport) std::string Base64Decoder(char* data, int length);