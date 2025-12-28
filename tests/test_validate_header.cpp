#include <iostream>
#include <fix/core/Validator.hpp>
#include <fix/error/utility.hpp>
#include <fix/message/GenericMessage.hpp>

Fix::Message::GenericMessage validheader1msg = {
    {8, "FIX.4.4"},
    {9, "112"},
    {35, "D"},
    {49, "BUY_SIDE"},
    {56, "SELL_SIDE"},
    {34, "1"},
    {52, "20251202-12:00:00.000"}
};

Fix::Message::GenericMessage validheader2msg = {
    {8, "FIX.4.4"},
    {9, "145"},
    {35, "8"},
    {49, "EXEC_BROKER"},
    {56, "CLIENT_FIRM"},
    {115, "TP_COMPANY"},
    {128, "DEST_COMPANY"},
    {34, "42"},
    {50, "TRADER_1"},
    {57, "DESK_A"},
    {142, "LON-DESK1"},
    {143, "NY-DESK2"},
    {52, "20251202-12:05:30.123"}
};


Fix::Message::GenericMessage validheader3msg = {
    {8, "FIX.4.4"},
    {9, "250"},
    {35, "D"},
    {49, "ORIG_FIRM"},
    {56, "DEST_FIRM"},
    {115, "TP_COMPANY"},
    {627, "2"},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
    {34, "10"},
    {52, "20251202-12:15:45.555"},
    {212, "24"},
    {213, "<Fixml>...</Fixml>"}
};

Fix::Message::GenericMessage invalidheader1msg = {
    {8, "FIX.4.2"}, // wrong version
    {9, "250"},
    {35, "D"},
    {49, "ORIG_FIRM"},
    {56, "DEST_FIRM"},
    {115, "TP_COMPANY"},
    {627, "2"},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
    {34, "10"},
    {52, "20251202-12:15:45.555"},
    {212, "24"},
    {213, "<Fixml>...</Fixml>"}
};

Fix::Message::GenericMessage invalidheader2msg = {
    {8, "FIX.4.4"},
    {9, "TWO_FIFTY"}, // wrong body length type
    {35, "D"},
    {49, "ORIG_FIRM"},
    {56, "DEST_FIRM"},
    {115, "TP_COMPANY"},
    {627, "2"},
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
    {34, "10"},
    {52, "20251202-12:15:45.555"},
    {212, "24"},
    {213, "<Fixml>...</Fixml>"}
};  

Fix::Message::GenericMessage invalidheader3msg = {
    {8, "FIX.4.4"},
    {9, "250"},
    {35, "X"},// wrong msg type
    {49, "ORIG_FIRM"},
    {56, "DEST_FIRM"},
    {115, "TP_COMPANY"},
    {627, "1"}, // undersized group count
    {628, "HOP1_COMP"},
    {629, "20251202-12:00:01.000"},
    {630, "1001"},
    {628, "HOP2_COMP"},
    {629, "20251202-12:00:02.000"},
    {630, "1002"},
    {34, "10"},
    {52, "20251202-12:15:45.555"},
    {212, "24"},
    {213, "<Fixml>...</Fixml>"}
};

int main() {

    Fix::Validator v{};
    std::string expected_msg_type1 = "D";
    v.validate_header_(validheader1msg, expected_msg_type1);
    if (res1.empty()) {
        std::cout << "validheader1 passed validation\n";
    } else {                
        std::cout << "validheader1 failed validation\n";  
    }   
    std::string expected_msg_type2 = "8";
    auto res2 = v.validate_header_(validheader2msg, expected_msg_type2);
    if (res2.empty()) {
        std::cout << "validheader2 passed validation\n";
    } else {                
        std::cout << "validheader2 failed validation\n";  
    }   
    std::string expected_msg_type3 = "D";
    auto res3 = v.validate_header_(validheader3msg, expected_msg_type3);
    if (res3.empty()) {
        std::cout << "validheader3 passed validation\n";
    } else {        
        std::cout << "validheader3 failed validation\n";
    }

    std::string expected_msg_type4 = "D";
    auto res4 = v.validate_header_(invalidheader1msg, expected_msg_type4);
    if (res4.empty()) {
        std::cout << "invalidheader1 passed validation\n";
    } else {        
        std::cout << "invalidheader1 failed validation\n";  
    }           

    std::string expected_msg_type5 = "D";
    auto res5 = v.validate_header_(invalidheader2msg, expected_msg_type5);      
    if (res5.empty()) {
        std::cout << "invalidheader2 passed validation\n";
    } else {        
        std::cout << "invalidheader2 failed validation\n";  
    }

    std::string expected_msg_type6 = "D";
    auto res6 = v.validate_header_(invalidheader3msg, expected_msg_type6);
    if (res6.empty()) {
        std::cout << "invalidheader3 passed validation\n";
    } else {        
        std::cout << "invalidheader3 failed validation\n";  
    }
    return 0;
}