#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace Fix::Log {
    struct LogFile {
        std::ofstream stream;
        std::string path;
        std::chrono::steady_clock::time_point last_used;
        bool is_open() const noexcept {return stream.is_open();}


    };
}