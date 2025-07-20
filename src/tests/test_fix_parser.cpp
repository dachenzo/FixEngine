#include <utility>
#include <string>
#include <iostream>
#include <vector>
#include <string_view>
#include "../fix/fix_parser.hpp"






int main() {
    const std::string test_message =
        "8=FIX.4.4\x01"               // BeginString
        "9=67\x01"                     // BodyLength
        "35=A\x01"                     // MsgType=Logon
        "49=SENDER\x01"                // SenderCompID
        "56=TARGET\x01"                // TargetCompID
        "34=1\x01"                     // MsgSeqNum
        "108=30\x01"                   // HeartBtInt=30s
        "98=0\x01"                     // EncryptMethod=None
        "52=20250720-12:34:56.000\x01" // SendingTime (UTC)
        "10=130\x01" 
        "8=FIX.4.4\x01"                       // BeginString
        "9=112\x01"                           // BodyLength
        "35=D\x01"                            // MsgType = NewOrderSingle
        "49=CLIENT12\x01"                     // SenderCompID
        "56=BROKER12\x01"                     // TargetCompID
        "34=2\x01"                            // MsgSeqNum
        "52=20250720-12:45:00.000\x01"        // SendingTime
        "11=12345\x01"                        // ClOrdID
        "21=1\x01"                            // HandlInst = Automated, no broker intervention
        "55=IBM\x01"                          // Symbol
        "54=1\x01"                            // Side = Buy
        "38=100\x01"                          // OrderQty
        "40=2\x01"                            // OrdType = Limit
        "44=125.50\x01"                       // Price
        "59=0\x01"                            // TimeInForce = Day
        "10=209\x01";                         // CheckSum
        
  
    Fix::Parser parser{};

    std::vector<std::pair<std::size_t, std::size_t>> ranges{{0, 10},{10, 20},{20, 30},{30, 40},{40, 65}, {65, 89}, {89, 120},{120, 145},{145, 170},{170, 224}};
    std::vector<std::string_view> views;

    

    for (auto& p: ranges) {
        views.push_back(std::string_view{test_message.data()+p.first, p.second-p.first});
    }

    for (auto& v: views) {
        auto res = parser.parse(v);
        if (res.has_value()) {
            std::cout << "NEW MESSAGE" << '\n';
            for (auto c: res.value()) {
                std::cout << c.tag << '=' << c.value << '\n';
            }
        }
    }

   

   



    
}

