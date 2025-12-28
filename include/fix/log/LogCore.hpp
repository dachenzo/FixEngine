#pragma once
#include <thread>
#include <semaphore>
#include <fix/log/LogEntry.hpp>
#include <fix/log/LogFileSink.hpp>
#include <fix/log/MpscRing.hpp>
#include <fix/core/definitions.hpp>

namespace Fix::Log {
    
    struct LogCore {
        LogCore(std::string&& engine_run_id);
        LogCore(const LogCore& other) = delete;
        LogCore& operator=(const LogCore& other) = delete;
        LogCore(const LogCore&& other) = delete;
        LogCore& operator=(const LogCore&& other) = delete;
        ~LogCore();

        void push(Log::Entry&& entry);

        bool try_push(Log::Entry&& entry);

        void add_session(Fix::SessionID& id, std::string&& readable_id);


        

        private:
        MpscRing<Log::Entry> q_;
        Log::LogFileSink file_sink_;
        std::jthread t_;
        std::counting_semaphore<MpscRing<Log::Entry>::capacity> sem_{0};

        void drain(std::stop_token st);

        


    };
}