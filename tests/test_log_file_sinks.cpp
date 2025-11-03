#include <fix/log/LogFileSink.hpp>
#include <fix/log/LogEntry.hpp>
#include <vector>
#include <fix/definitions.hpp>

int main() {

    Fix::Log::LogFileSink log_sink{};

    std::size_t session_counter = 10;
    int write_loop = 20;

    std::vector<Fix::SessionID> ids;

    for (std::size_t i = 0; i < session_counter; i++) {
        Fix::SessionID sid{
            .storage_index = i,
            .id = i + 1000u
        };
        log_sink.add_session(sid);
        ids.push_back(sid);
    }

    for (int i = 0; i < write_loop; i++)
    for (std::size_t i = 0; i < session_counter; i++) {
        Fix::Log::Entry entry {
            .message = "Test entry !!:)",
            .sess_id = ids[i],
            .context {
                .layer = Fix::Error::Layer::Peripheral,
                .cat = Fix::Error::Category::Debug,
                .sev = Fix::Error::Severity::NA
            }
            
        };

        log_sink.write(entry);
    }



}


