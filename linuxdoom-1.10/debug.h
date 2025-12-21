#ifndef DEBUG_H
#define DEBUG_H

#if defined(__ANDROID__)
    #include <android/log.h>

    #ifdef NDEBUG
        #define LOG(...)
    #else
        #define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "flutter", __VA_ARGS__)
    #endif
#elif defined(__APPLE__) || defined(TARGET_OS_IPHONE) || defined(__linux__)
    #ifdef NDEBUG
        #define LOG(...)
    #else
        #define LOG(...) printf(__VA_ARGS__)
    #endif
#endif

#endif