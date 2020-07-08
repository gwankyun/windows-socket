#pragma once
#include <cstdio>
#include <cstring>
#include <cstdarg>

inline void logPrintf(const char* func, int line, int funcLen, const char* format, ...)
{
    char buffer[256];
    memset(buffer, '\0', sizeof(buffer));

    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, sizeof(buffer), format, ap);
    va_end(ap);

    char out[512];
    memset(out, '\0', sizeof(out));
    sprintf_s(out, sizeof(out), "[%-*.*s:% 4d] %s\n", funcLen, funcLen, func, line, buffer);
    printf("%s", out);
}

inline void logPrintf(const char* func, int line, int funcLen)
{
    logPrintf(func, line, funcLen, "%s", "");
}

#define LOG(...) \
    logPrintf(__func__, __LINE__, 15, ##__VA_ARGS__)
