#include <fix/log/LogCore.hpp>



namespace Fix::Log {
    LogCore::LogCore(std::string&& engine_run_id): q_{}, file_sink_{std::forward<std::string>(engine_run_id)} {

    }

    void LogCore::push(Log::Entry&& entry) {
        q_.push(entry);
    }


    bool LogCore::try_push(Log::Entry&& entry) {
        return q_.try_push(entry);
    }

    void LogCore::add_session(Fix::SessionID& id) {
        file_sink_.add_session(id);
    }

    void LogCore::drain() {
        int cnt = 0; 
        for (;;) {
            while (cnt < 64) {
                cnt++;
            }
        }
    }
    
}