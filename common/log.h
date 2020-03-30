#ifndef LOG_H
#define LOG_H

#include <android/log.h>
#define L(...) do { __android_log_print(ANDROID_LOG_DEBUG, "vncd", __VA_ARGS__); printf(__VA_ARGS__); } while (0)

#endif // LOG_H
