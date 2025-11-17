#include <string_view>
#include <charconv>
#include <fix/core/definitions.hpp>
#include <fix/core/utils.hpp>
    

namespace Fix::Utils {

        inline std::string_view role_to_string(Fix::Role role) noexcept {
            switch (role)
            {
                using namespace std::string_view_literals;
                case Fix::Role::INITIATOR: return "Initiator"sv;
                case Fix::Role::ACCEPTOR: return "Acceptor"sv;
                default: return "Unknown role"sv;
            }
        }

   
    
}