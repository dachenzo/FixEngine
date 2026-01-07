#pragma once
#include <cstdlib>
#include <string>
#include <cstdint>
#include <optional>

namespace Fix {

    const std::string DEFAULT_FIX_VERSION = "FIX.4.4";
    
    struct SessionID {
        std::size_t storage_index;
        std::size_t id;

        bool operator==(const Fix::SessionID& other) const noexcept {
            return other.id == this->id;
        }

        bool operator!=(const Fix::SessionID& other) const noexcept {
            return other.id != this->id;
        }
    };

    enum class Role {
        INITIATOR,
        ACCEPTOR
    };

    struct ConnectionConfig {
        std::string ip;
        uint16_t port;
        Fix::Role role;
        int backlog;
    }; 

    struct SessionParameters {
        std::string fix_version = "FIX.4.4";
        std::string sender_comp_id = "test_sender";
        std::string target_comp_id = "test_target";
        std::optional<std::string> username;
        std::optional<std::string> password;
        std::string encrypt_method_str = "0"; // none
        std::string heart_beat_str = "30";
        int heart_beat_int = 30;
        int encrypt_method = 0;
        bool reset_on_logon = true;
        bool send_last_msg_prcessed_seq =true;
        
    }; 

    struct SessionCreationConfig {
        Fix::Role role;
        Fix::ConnectionConfig conn_config;
        Fix::SessionParameters params;
    };

    

    struct AcceptorPolicy {
        bool allow_reset_on_logon = true;     // honor 141=Y from peer
        int  min_heart_bt = 5;                // clamp peer 108
        int  max_heart_bt = 120;
        bool require_credentials = false;     // if true, 553/554 must be present & valid
};


    enum class TimerType {};

}

namespace std {
    template<>
    struct hash<Fix::SessionID> {
        size_t operator()(const Fix::SessionID& id) const noexcept {
            return std::hash<std::size_t>{}(id.id);
        }
    };























} 