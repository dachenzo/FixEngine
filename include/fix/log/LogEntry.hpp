#pragma once
#include <string>
#include <sstream>
#include <fix/core/definitions.hpp>
#include <fix/log/LogContext.hpp>
#include <fix/error/utility.hpp>

namespace Fix::Log
{
    struct Entry {
        std::string message;
        Fix::SessionID sess_id;
        Fix::Log::Context context; 


        std::string to_json() const {
            std::ostringstream oss;
            oss << "{"
                << "\"Category\":\"" << Fix::Error::to_string(context.cat) << "\","
                << "\"Layer\":\"" << Fix::Error::to_string(context.layer) << "\","
                << "\"Severity\":\"" << Fix::Error::to_string(context.sev) << "\","
                << "\"Message\":\"" << message << "\""
                << "}";
            return oss.str();
        }
    };
} // namespace Fix::Log
