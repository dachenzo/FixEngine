#include <fix/log/LogCore.hpp>



namespace Fix::Log {
    LogCore::LogCore(): q_{} {

    }

    void LogCore::push(Log::Entry& entry) {
        q_.push(entry);
    }


    bool LogCore::try_push(Log::Entry& entry) {
        return q_.try_push(entry);
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