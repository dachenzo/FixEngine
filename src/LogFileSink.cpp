#include <iostream>
#include <exception>
#include <fix/log/LogFileSink.hpp>



namespace Fix::Log {

    LogFileSink::LogFileSink(std::string&& engine_run_id): engine_run_id_{engine_run_id} {
        core_log_file_.path = LogFileSink::make_path("logs", engine_run_id_, "core_log.json");
        open_file(core_log_file_);
    }



    LogFileSink::~LogFileSink() {

        try {
            shutdown();
        }
        catch (std::exception& e) {
            // suppress all exceptions
            std::cerr << "Exception during LogFileSink shutdown: " << e.what() << "\n";
        }
    }

    void LogFileSink::shutdown() {
        for (auto& [sid, node] : sess_to_file_) {
            close_file(node.file);
        }

    }


    void LogFileSink::write(const Log::Entry& entry) {
        auto& node = get_node(entry.sess_id);
        ensure_open(entry.sess_id, node);
        node.file.stream << entry.to_json() << std::endl; // might want to switch back to non forced flushing
    }

    void LogFileSink::add_session(Fix::SessionID& sess_id) {
        Log::SinkNode node = create_sink_node(sess_id);
        sess_to_file_.emplace(std::move(sess_id), std::move(node));
        Log::Entry entry {
            .message = "Added session to LogFileSink",
            .sess_id = sess_id,
            .context = {
                .layer = Fix::Error::Layer::Peripheral,
                .cat = Fix::Error::Category::Info,
                .sev = Fix::Error::Severity::Low
            }
        };
        write(entry);
    }

    void LogFileSink::log_internal(const Log::Entry& entry) {
        if (core_log_file_.is_open()) {
            core_log_file_.stream << entry.to_json() << "\n";
        } else {
            // if core log file is not open, there's not much we can do
            std::cerr << "Core log file is not open. Log entry: " << entry.to_json() << "\n";
        }
    }

    Log::SinkNode LogFileSink::create_sink_node(Fix::SessionID& sess_id) {
        Log::LogFile log_file{};
        std::string file_name = std::to_string(sess_id.id);
        file_name.append(".json");
        log_file.path = std::move(LogFileSink::make_path("logs", engine_run_id_, "sessions", file_name));
        Log::SinkNode node{std::move(log_file), {}, false};
        return node;
    }

    SinkNode& LogFileSink::get_node(const Fix::SessionID& sess_id) {
        auto it = sess_to_file_.find(sess_id);

        if (it == sess_to_file_.end()) {
            Log::Entry entry {
                .message = "Attempted to log to unregistered session",
                .sess_id = sess_id,
                .context = {
                    .layer = Fix::Error::Layer::Peripheral,
                    .cat = Fix::Error::Category::Error,
                    .sev = Fix::Error::Severity::High
                }
            };
            log_internal(entry);


            // alternatively, throw or create a new node
            throw std::runtime_error("Attempted to log to unregistered session");
        }

        return it->second;
    }



    void LogFileSink::evict_one() {
        if (currently_open_files_ == 0) return;
        const auto& victim_sid = lru_cache_.back();
        auto& v = sess_to_file_.at(victim_sid);
        close_file(v.file);                // flush & close if fd
        v.in_lru = false;
        lru_cache_.pop_back();
        --currently_open_files_;
    }

    void LogFileSink::touch(const Fix::SessionID& sess_id, SinkNode& node) {
        if (!node.in_lru) {
            lru_cache_.push_front(sess_id);
            node.lru_iterator = lru_cache_.begin();
            node.in_lru = true;
            return ;
        }

        lru_cache_.splice(lru_cache_.begin(), lru_cache_, node.lru_iterator);
    }
  
    void LogFileSink::ensure_open(const Fix::SessionID& sess_id, SinkNode& node) {
        if (!node.file.is_open()) {
            if (currently_open_files_ >= MAX_LRU_CACHE_SIZE) evict_one();
            open_file(node.file); 
            
            if (!node.file.is_open()) {
                return;
            }
            ++currently_open_files_;
        }

        touch(sess_id, node);
    }


    void LogFileSink::open_file(Log::LogFile& file) {
        namespace fs = std::filesystem;
        fs::create_directories(fs::path(file.path).parent_path());

        file.stream.open(file.path, std::ios::out | std::ios::app | std::ios::binary);

        if (!file.is_open()) {
            Log::Entry entry {
                .message = "Failed to open log file: " + file.path,
                .sess_id = {.storage_index = 0, .id = 0},
                .context = {
                    .layer = Fix::Error::Layer::Peripheral,
                    .cat = Fix::Error::Category::Error,
                    .sev = Fix::Error::Severity::High
                }
            };

            log_internal(entry);
            
        }

    }

    void LogFileSink::close_file(Log::LogFile& file) {

        if (file.is_open()) {
            file.stream.flush();
            file.stream.close();
        }

    }

}