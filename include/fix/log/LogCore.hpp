#pragma once
#include <fix/log/LogEntry.hpp>

namespace Fix::Log {
    
    struct LogCore {
        LogCore();
        LogCore(const LogCore& other) = delete;
        LogCore& operator=(const LogCore& other) = delete;
        LogCore(const LogCore&& other) = delete;
        LogCore& operator=(const LogCore&& other) = delete;

        void push(Log::Entry& entry);

        


    };
}