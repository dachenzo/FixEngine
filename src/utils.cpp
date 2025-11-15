#include <string_view>
#include <charconv>
#include <fix/definitions.hpp>
#include <fix/utils.hpp>


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

        inline bool parse_int(std::string_view sv, std::size_t& out) {
            int tmp = 0;
            auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), tmp);
            if (ec != std::errc() || ptr != sv.data() + sv.size() || tmp < 0) {
                return false;
            }
            out = static_cast<std::size_t>(tmp);
            return true;
        }

        

    
}