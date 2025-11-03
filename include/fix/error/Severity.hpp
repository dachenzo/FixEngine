#pragma once
#include <string>

namespace Fix::Error {

    enum class Severity {
        NA,
        Low,
        Moderate,
        High,
        Fatal
    };

    std::string to_string(Error::Severity sev) {
  
        switch (sev)
        {
        case Severity::NA:
            return "N/A";
        
        case Severity::Low:
            return "low";      

        case Severity::Moderate:
            return "moderate";

        case Severity::High:
            return "high";

        case Severity::Fatal:
            return "fatal";
        default:
            return "unknown error severity";
        }
    }
}