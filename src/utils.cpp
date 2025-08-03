#include "Config.hpp"


#ifdef PLATFORM_WIN

#define CONSOLE_COLOR_RESET ""
#define CONSOLE_COLOR_GREEN ""
#define CONSOLE_COLOR_RED ""
#define CONSOLE_COLOR_PURPLE ""


#else

#define CONSOLE_COLOR_RESET "\033[0m"
#define CONSOLE_COLOR_GREEN "\033[1;32m"
#define CONSOLE_COLOR_RED "\033[1;31m"
#define CONSOLE_COLOR_PURPLE "\033[1;35m"


#endif

#define MAX_TEXT_BUFFER_LENGTH 512

static void LogMessage(int level, const char *msg, va_list args)
{
    time_t rawTime;
    struct tm *timeInfo;
    char timeBuffer[80];


    time(&rawTime);
    timeInfo = localtime(&rawTime);

    strftime(timeBuffer, sizeof(timeBuffer), "[%H:%M:%S]", timeInfo);

    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), msg, args);

    switch (level)
    {
        case 0:
            printf("%s%s: %s%s\n", CONSOLE_COLOR_GREEN, timeBuffer, buffer,
                   CONSOLE_COLOR_RESET);
            break;
        case 1:
            printf("%s%s: %s%s\n", CONSOLE_COLOR_RED, timeBuffer, buffer,
                   CONSOLE_COLOR_RESET);
            break;
        case 2:
            printf("%s%s: %s%s\n", CONSOLE_COLOR_PURPLE, timeBuffer, buffer,
                   CONSOLE_COLOR_RESET);
            break;
    }
}


void LogError(const char *msg, ...)
{


    va_list args;
    va_start(args, msg);
    LogMessage(1, msg, args);
    va_end(args);
}


void LogWarning(const char *msg, ...)
{

    va_list args;
    va_start(args, msg);
    LogMessage(2, msg, args);
    va_end(args);
}


void LogInfo(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    LogMessage(0, msg, args);
    va_end(args);
}

std::vector<std::string> split(const char *str, char separator,
                               bool keepEmptyStrings = false)
{
    std::vector<std::string> ret;
    const char *strEnd = str + std::strlen(str);

    for (const char *splitEnd = str; splitEnd != strEnd; ++splitEnd)
    {
        if (*splitEnd == separator)
        {
            const u64 splitLen = splitEnd - str;
            if (splitLen > 0 || keepEmptyStrings)
            {
                char buffer[256];
                memcpy(buffer, str, splitLen);
                buffer[splitLen] = '\0';
                ret.push_back(buffer);
            }
            str = splitEnd + 1;
        }
    }

    const u64 splitLen = strEnd - str;
    if (splitLen > 0 || keepEmptyStrings)
    {
        char buffer[256];
        memcpy(buffer, str, splitLen);
        buffer[splitLen] = '\0';
        ret.push_back(buffer);
    }


    return ret;
}