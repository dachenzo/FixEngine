#pragma once
#include <string>

#include <fix/definitions.hpp>
#include <fix/log/LogContext.hpp>

namespace Fix::Log
{
    struct Entry {
        std::string message;
        Fix::SessionID sess_id;
        Fix::Log::Context context; 
    };
} // namespace Fix::Log
