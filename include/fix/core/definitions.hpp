#pragma once
#include <cstdlib>
#include <functional>
#include <string>
#include <cstdint>
#include <array>
#include <optional>
#include <sys/types.h>

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

    using ReconnectCallback = std::function<void(const Fix::SessionID)>;

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
        std::array<std::string, 1> no_resend_messages_types = {"0"}; // types that are not subject to resend requests
        std::string fix_version = "FIX.4.4";
        std::string sender_comp_id = "test_sender";
        std::string target_comp_id = "test_target";
        std::optional<std::string> username;
        std::optional<std::string> password;
        u_int32_t heart_beat_int = 30;
        int encrypt_method = 0;
        bool initiator_reset_on_logon = false;
        bool acceptor_reset_on_logon = true;
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


    

}

namespace std {
    template<>
    struct hash<Fix::SessionID> {
        size_t operator()(const Fix::SessionID& id) const noexcept {
            return std::hash<std::size_t>{}(id.id);
        }
    };

} 