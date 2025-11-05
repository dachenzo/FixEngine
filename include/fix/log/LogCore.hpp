#pragma once
#include <thread>
#include <fix/log/LogEntry.hpp>
#include <fix/log/LogFileSink.hpp>
#include <fix/log/MpscRing.hpp>
#include <fix/definitions.hpp>

namespace Fix::Log {
    
    struct LogCore {
        LogCore(std::string&& engine_run_id);
        LogCore(const LogCore& other) = delete;
        LogCore& operator=(const LogCore& other) = delete;
        LogCore(const LogCore&& other) = delete;
        LogCore& operator=(const LogCore&& other) = delete;

        void push(Log::Entry&& entry);

        bool try_push(Log::Entry&& entry);

        void add_session(Fix::SessionID& id);


        

        private:
        MpscRing<Log::Entry> q_;
        Log::LogFileSink file_sink_;
        std::thread t_;

        void drain();

        


    };
}