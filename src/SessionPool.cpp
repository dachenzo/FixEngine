#include <optional>
#include <vector>
#include <span>
#include <algorithm>
#include <fix/SessionPool.hpp>



namespace Fix {
    SessionPool::SessionPool() {
        sessions_.reserve(1024);
        free_indices_.reserve(1024);
    }

    std::size_t SessionPool::size() noexcept {
        return sessions_.size()-free_indices_.size();
    }


    void SessionPool::emplace_session(           
                Fix::Role role,
                std::unique_ptr<IConnection> conn,
                Fix::Application& app,
                Fix::ITimerFactory& timers
    ) {
        if (free_indices_.empty()) {
            Fix::SessionID session_id = {sessions_.size(), generate_session_Id_()};
            sessions_.emplace_back(
                std::make_shared<Session>(
                    session_id,
                    role,
                    std::move(conn),
                    app,
                    timers
                )
            );
        } else {
            std::size_t nxt = free_indices_.back();
            free_indices_.pop_back();
            Fix::SessionID session_id = {nxt, generate_session_Id_()};
            sessions_[nxt] = std::make_shared<Fix::Session>(
                session_id,
                role,
                std::move(conn),
                app,
                timers
            );
            
        }
    }

    bool SessionPool::remove(Fix::SessionID id) {
        auto storage_index = id.storage_index;
        if (sessions_.at(storage_index)->get_session_id() != id) {
            return false;
        }
        
        free_indices_.push_back(storage_index);
        return true;
    }

    std::shared_ptr<Fix::Session> SessionPool::get(Fix::SessionID id) {
        auto storage_index = id.storage_index;
        if (sessions_.at(storage_index)->get_session_id() != id) {
            return nullptr;
        }

        return sessions_.at(storage_index);
    }

    std::span<std::shared_ptr<Fix::Session>> SessionPool::get_all_at_start() {
        return std::span{sessions_.begin(), sessions_.end()};
    }

    std::vector<std::shared_ptr<Fix::Session>> SessionPool::get_all() {
        std::vector<std::shared_ptr<Fix::Session>> result(size());
        auto& free = free_indices_;
        for (auto& session: sessions_) {
            auto it = std::find(free.begin(), free.end(), session->get_session_id().storage_index);
            if (it == free.end()) {
                result.push_back(session);
            }
        }
        return result;
    }


    std::size_t SessionPool::generate_session_Id_() noexcept {
        return next_idx++;
    }
        
}