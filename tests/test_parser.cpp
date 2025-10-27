#include <fix/Parser.hpp>
#include <iostream>
#include <vector>
#include <string_view>
#include <string>

int main() {
    Fix::Parser p{};

    const std::string kFixLogon =
        "8=FIX.4.4\x01"
        "9=77\x01"
        "35=A\x01"
        "34=1\x01"
        "49=CLIENT12\x01"
        "52=20251007-15:42:39.255\x01"
        "56=EXECUTOR\x01"
        "98=0\x01"
        "108=30\x01"
        "141=Y\x01"
        "10=199\x01";

    // Feed in chunks: [start, end)
    const std::vector<std::pair<size_t, size_t>> ranges{
        {0, 14},
        {14, 30},
        {30, 33},
        {33, 45},
        {45, 70},
        {70, 75},
        {75, 85},
        {85, 91},
        {91, 99},
        {99, kFixLogon.size()}
    };

    std::optional<Fix::Message> res;
    for (auto [lo, hi] : ranges) {
        std::string_view sv{kFixLogon.data() + lo, hi - lo};
        res = p.parse(sv);
        if (res.has_value()) break;
    }

    if (res) std::cout << "PARSE SUCCEEDED\n";
    else     std::cout << "PARSE FAILED\n";
    
}
