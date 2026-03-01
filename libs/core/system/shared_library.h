#pragma once

// Platform-specific export macros
#if defined(_WIN32) || defined(_WIN64)
    #define LIB_EXPORT __declspec(dllexport)
    #define LIB_IMPORT __declspec(dllimport)
#else
    #define LIB_EXPORT __attribute__((visibility("default")))
    #define LIB_IMPORT
#endif
