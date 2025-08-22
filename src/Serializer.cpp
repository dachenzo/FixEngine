#include <charconv>
#include <cstdlib>
#include <fix/Serializer.hpp>



namespace Fix {

    inline void append_uint(int x, std::string& buff) {
        char tmp[32];
        auto [p, ec] = std::to_chars(std::begin(tmp), std::end(tmp), x);
        buff.append(tmp, p);
    }

     std::size_t Serializer::serialize(Fix::Message& msg, std::string& buff) {
        buff.clear();
        constexpr char SOH = '\x01';
        for (auto& field: msg.get_fields()) {
            append_uint(field.tag, buff);
            buff.push_back('=');
            buff.append(field.value);
            buff.push_back(SOH);
        }

        return buff.size();
    }
    
}




