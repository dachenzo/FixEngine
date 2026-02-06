#pragma once
#include <list>
#include <cstdlib>
#include <unordered_map>
#include <filesystem>
#include <fix/core/definitions.hpp>
#include <fix/log/LogFile.hpp>
#include <fix/log/LogEntry.hpp>

namespace Fix::Log
{
    struct SinkNode {
        Log::LogFile file;
        std::list<Fix::SessionID>::iterator lru_iterator;
        bool in_lru = false;
        bool first_write = true;
    }; 
    
    
    struct LogFileSink {

        LogFileSink(std::string&& engine_run_id);
        LogFileSink(const LogFileSink& other) = delete;
        LogFileSink& operator=(const LogFileSink& other) = delete;
        LogFileSink(const LogFileSink&& other) = delete;
        LogFileSink& operator=(const LogFileSink&& other) = delete;
        ~LogFileSink();

        static constexpr std::size_t MAX_LRU_CACHE_SIZE = 128;

        template <typename... Args>
        static std::string make_path(Args&& ...parts) {
            std::filesystem::path p;
            ((p/= std::forward<Args>(parts)), ...);
            return p.string();
        };  

        void write (const Log::Entry& entry);

        void shutdown();

        void add_session(Fix::SessionID& id,  std::string && readable_id);

        private:
        std::list<Fix::SessionID> lru_cache_;
        std::unordered_map<Fix::SessionID, Log::SinkNode> sess_to_file_;
        Log::LogFile core_log_file_;
        std::string engine_run_id_;
        std::size_t currently_open_files_ = 0;


        SinkNode& get_node(const Fix::SessionID& sid);

        void ensure_open(const Fix::SessionID& sid, SinkNode& n);

        void evict_one();

        void touch(const Fix::SessionID& sid, SinkNode& n);

        void open_file(Log::LogFile& file);

        void close_file(Log::SinkNode& node);

        void log_internal(const Log::Entry& entry);



        Log::SinkNode create_sink_node(Fix::SessionID& sess_id,  std::string && readable_id);

        


    };
} // namespace fix::log