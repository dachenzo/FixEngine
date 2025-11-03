#pragma once
#include <thread>
#include <fix/log/LogEntry.hpp>
#include <fix/log/LogFileSink.hpp>
#include <fix/log/MpscRing.hpp>

namespace Fix::Log {
    
    struct LogCore {
        LogCore();
        LogCore(const LogCore& other) = delete;
        LogCore& operator=(const LogCore& other) = delete;
        LogCore(const LogCore&& other) = delete;
        LogCore& operator=(const LogCore&& other) = delete;

        void push(Log::Entry& entry);

        bool try_push(Log::Entry& entry);


        

        private:
        MpscRing<Log::Entry> q_;
        Log::LogFileSink file_sink_;
        std::thread t_;

        void drain();

        


    };
}