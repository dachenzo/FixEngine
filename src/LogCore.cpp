#include <fix/log/LogCore.hpp>



namespace Fix::Log {
    LogCore::LogCore(std::string&& engine_run_id): q_{}, file_sink_{std::forward<std::string>(engine_run_id)} {
        t_ = std::jthread([this](std::stop_token st){ drain(st); });
    }

    void LogCore::push(Log::Entry&& entry) {
        q_.push(std::move(entry));
        sem_.release(1);
    }

    LogCore::~LogCore() {
        if (t_.joinable()) {
            t_.request_stop(); 
            sem_.release(MpscRing<Log::Entry>::capacity);
            t_.join();
        }
        else {
            file_sink_.shutdown();
        }
        
    }


    bool LogCore::try_push(Log::Entry&& entry) {
        if (q_.try_push(std::move(entry))) {
            sem_.release(1);
            return true;
        }

        return false;
    }

    void LogCore::add_session(Fix::SessionID& id, std::string && readable_id) {
        file_sink_.add_session(id, std::move(readable_id));
    }

    void LogCore::drain(std::stop_token st) {
        Log::Entry entry;


        while (!st.stop_requested()) {
            sem_.acquire();

            int drained = 0;

            do {
                while (q_.pop(entry)) {
                    file_sink_.write(entry);
                    ++drained;

                    if (drained >= 1024) break; // cap burst
                }
            } while (sem_.try_acquire());
        }

        int cnt = 0;

        while (cnt < MpscRing<Log::Entry>::capacity) {
            if (q_.pop(entry)) {
                file_sink_.write(entry);
            }
            cnt++;
        }
        file_sink_.shutdown();
    }
    
}