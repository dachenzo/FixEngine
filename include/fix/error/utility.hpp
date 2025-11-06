#pragma once
#include <fix/error/Severity.hpp>
#include <fix/error/Category.hpp>
#include <fix/error/Layer.hpp>
#include <string>

namespace Fix::Error {

    inline std::string to_string(Severity sev) {
        switch (sev) {
            case Severity::NA: return "NA";
            case Severity::Low: return "Low";
            case Severity::Moderate: return "Moderate";
            case Severity::High: return "High";
            case Severity::Fatal: return "Fatal";
            default: return "Unknown";
        }
    }

    inline std::string to_string(Category cat) {
        switch (cat) {
            case Category::Debug: return "Debug";
            case Category::Info: return "Info";
            case Category::Warn: return "Warn";
            case Category::Error: return "Error";
            default: return "Unknown";
        }
    }

    inline std::string to_string(Layer layer) {
        switch (layer) {
            case Layer::Transport: return "Transport";
            case Layer::Fix: return "Fix";
            case Layer::App: return "App";
            case Layer::Peripheral: return "Peripheral";
            default: return "Unknown";
        }
    }

} // namespace Fix::Error