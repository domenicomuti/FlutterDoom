#ifndef DEBUG_H
#define DEBUG_H

#if defined(__ANDROID__)
    #include <android/log.h>

    #ifdef NDEBUG
        #define LOG(...)
    #else
        #define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "flutter", __VA_ARGS__)
    #endif
#else
    #ifdef NDEBUG
        #define LOG(...)
    #else
        #define LOG(...) printf(__VA_ARGS__)
    #endif
#endif

#endif