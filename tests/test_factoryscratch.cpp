#include <gtest/gtest.h>
#include <vector>
#include <fix/core/MessageFactory.hpp>



TEST(FactoryScratchTests, AddFieldTest) {
    Fix::FactoryScratch scratch;
    scratch.reset();
    scratch.add_field(35, "D");
    scratch.add_field(49, "SENDER");
    scratch.add_field(56, "TARGET");

    const char expected[] = "35=D"
                            "\x01"
                            "49=SENDER"
                            "\x01"
                            "56=TARGET"
                            "\x01";
    std::string_view result = scratch.get_buffer_view();
    std::string_view expected_sv{expected, sizeof(expected) - 1};
    EXPECT_EQ(result, expected_sv);
}

TEST(FactoryScratchTests, BodyLengthPlaceholderTest) {
    Fix::FactoryScratch scratch;
    scratch.reset();
    scratch.add_body_length_placeholder();

    const char expected[] = "9=000000"
                            "\x01";
    std::string_view result = scratch.get_buffer_view();
    std::string_view expected_sv{expected, sizeof(expected) - 1};
    EXPECT_EQ(result, expected_sv);
}

TEST(FactoryScratchTests, AddIntTest) {
    Fix::FactoryScratch scratch;
    scratch.reset();
    scratch.add_int(1234567890);

    const char expected[] = "1234567890";
    std::string_view result = scratch.get_buffer_view();
    std::string_view expected_sv{expected, sizeof(expected) - 1};
    EXPECT_EQ(result, expected_sv);
}

TEST(FactoryScratchTests, GrowTest) {
    Fix::FactoryScratch scratch;
    scratch.reset();
    std::size_t initial_size = scratch.get_buffer_size();
    for (int i = scratch.get_position(); i < initial_size; ++i) {
        scratch.add_char('A');
    }

    EXPECT_EQ(scratch.get_buffer_size(), initial_size);
    scratch.add_char('B'); // This should trigger a grow
    EXPECT_EQ(scratch.get_buffer_size(), initial_size + Fix::FactoryScratch::min_grow_size);
    EXPECT_EQ(scratch.get_buffer_view().back(), 'B');
    
}


TEST(FactoryScratchTests, ComputeBodyLengthTest) {
    Fix::FactoryScratch scratch;
    struct Field {
        int64_t tag;
        std::string value;
    };
    std::vector<Field> fields = {
        {35, "D"},
        {49, "SENDER"},
        {56, "TARGET"},
    };

    scratch.reset();
    scratch.add_body_length_placeholder();
    std::size_t expected_body_length = 0;
    for (const auto& field : fields) {
        scratch.add_field(field.tag, field.value);
        expected_body_length += std::to_string(field.tag).size() + 1 // tag + '='
                                + field.value.size() + 1; // value + SOH
    }   

    std::size_t body_length = scratch.compute_body_length();
    // Body length should be the total length of fields added
    std::size_t expected_length = scratch.get_position();
    EXPECT_EQ(body_length, expected_length);
    EXPECT_EQ(body_length, expected_body_length);
}

TEST(FactoryScratchTests, ComputeChecksumTest) {
    Fix::FactoryScratch scratch;
    struct Field {
        int64_t tag;
        std::string value;
    };
    std::vector<Field> fields = {
        {35, "D"},
        {49, "SENDER"},
        {56, "TARGET"},
    };
    std::size_t expected_checksum = 0;

    scratch.reset();
    for (const auto& field : fields) {
        scratch.add_field(field.tag, field.value);
        for (char c : std::to_string(field.tag)) {
            expected_checksum += static_cast<unsigned char>(c);
        }
        expected_checksum += static_cast<unsigned char>('='); // '='
        for (char c : field.value) {
            expected_checksum += static_cast<unsigned char>(c);
        }
        expected_checksum += static_cast<unsigned char>('\x01'); // SOH
    }
    std::size_t checksum = scratch.compute_checksum();
    EXPECT_EQ(checksum, expected_checksum % 256);

}


TEST(FactoryScratchTests, BodyLenghtAndChecksumIntegrationTest) {
    std::string expected_msg = 
        "8=FIX.4.4\x01"
        "9=25\x01"
        "35=D\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "10=197\x01";
    Fix::FactoryScratch scratch;
    struct Field {
        int64_t tag;
        std::string value;
    };
    scratch.reset();
    scratch.add_field(8, "FIX.4.4");
    scratch.add_body_length_placeholder();
    std::vector<Field> fields = {
        {35, "D"},
        {49, "SENDER"},
        {56, "TARGET"},
    };
    for (const auto& field : fields) {
        scratch.add_field(field.tag, field.value);
    }
    scratch.insert_body_length();
    scratch.insert_checksum();

    auto complete_msg = scratch.get_buffer_view();
    EXPECT_EQ(complete_msg, expected_msg);

}