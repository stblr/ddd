#include <common/Log.hh>

#include <common/Console.hh>
#include <payload/Lock.hh>
#include <payload/LogFile.hh>

extern "C" {
#include <stdio.h>
}

extern "C" void Log(LogLevel level, const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    VLog(level, format, vlist);
    va_end(vlist);
}

extern "C" void VLog(LogLevel level, const char *format, va_list vlist) {
    if (level == LOG_LEVEL_TRACE) {
        return;
    }

    va_list copy;
    va_copy(copy, vlist);
    vprintf(format, copy);

    va_copy(copy, vlist);
    LogFile::VPrintf(format, copy);

    Lock<NoInterrupts> lock;
    switch (level) {
    case LOG_LEVEL_ERROR:
        Console::s_fg = Console::Color::Red;
        break;
    case LOG_LEVEL_WARN:
        Console::s_fg = Console::Color::Yellow;
        break;
    case LOG_LEVEL_INFO:
        Console::s_fg = Console::Color::White;
        break;
    default:
        return;
    }
    Console::VPrintf(format, vlist);
}
