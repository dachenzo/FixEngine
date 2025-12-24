#include <iostream>
#include <fix/schema/Registry.hpp>



int main() {
    using namespace Fix::Schema;

    Registry reg{};

    auto logon = reg.get("A");
    if (logon) {
        std::cout << logon->msg_type << '\n';
    } else {
        std::cout << "Failed" << '\n';
    }
}