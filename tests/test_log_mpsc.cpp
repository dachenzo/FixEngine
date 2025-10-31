#include <vector>
#include <algorithm>
#include <thread>
#include <iostream>
#include <cassert>
#include <fix/log/MpscRing.hpp>



int main() {
    std::vector<std::thread> ts; ts.reserve(6);
    Fix::Log::MpscRing<int> ring{};
    std::vector<int> out;

    int total = 100;
    int thread_cnt = 5;

    for (int i = 0; i < thread_cnt; i++) {
        auto work = [i, &ring, &total]() {
            for (int start = i; start < total; start += 5) {
                ring.push(start);
            }
        };
        ts.emplace_back(work);
    }

    auto drain = [&] () {
        while (out.size() < total) {
            
            int n;
            if (ring.pop(n)) {
                out.push_back(n);
            }
        }
    };

    ts.emplace_back(drain);

    for (auto& t: ts) if (t.joinable()) t.join();

    std::sort(out.begin(), out.end());

    assert(out.size() == total);
    assert(out[0] == 0);
    for (int i = 1; i < out.size(); i++) {
        assert(out[i] - 1 == out[i-1]);
        std::cout << out[i] << '\n';
    }

}