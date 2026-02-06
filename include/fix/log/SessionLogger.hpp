#pragma once
#include <fix/log/LogCore.hpp>
#include <fix/log/LogEntry.hpp>

namespace Fix::Log
{
    struct SessionLogger {
        SessionLogger(SessionID& id, LogCore& core);
        SessionLogger(const SessionLogger& other) = delete;
        SessionLogger& operator=(const SessionLogger& other) = delete;
        SessionLogger(const SessionLogger&& other) = delete;
        SessionLogger& operator=(const SessionLogger&& other) = delete;

        void log(Log::Context context, const std::string& message) ;

        private:
        Fix::Log::LogCore& core_;
        Fix::SessionID sess_id_;


    };
} // namespace name