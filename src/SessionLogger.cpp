#include <fix/log/SessionLogger.hpp>


namespace Fix::Log
{
    SessionLogger::SessionLogger(SessionID& id, LogCore& core): sess_id_{id}, core_{core} {}

    void SessionLogger::log(Log::Context context, const std::string& message) {
        core_.push({std::move(message), sess_id_, context});
    }
} // namespace Fix::Log
