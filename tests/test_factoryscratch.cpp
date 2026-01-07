#include <gtest/gtest.h>
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