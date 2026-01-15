#include <gtest/gtest.h>
#include <fix/core/LinearBuffer.hpp>



TEST(LinearBuffer, BasicOperations) {
    Fix::LinearBuffer<char> buffer;

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readable_size(), 0);

    const char* data = "Hello, World!";
    buffer.push(std::span<const char>(data, strlen(data)));

    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.readable_size(), strlen(data));

    auto readable = buffer.readable();
    EXPECT_EQ(std::string(readable.data(), readable.size()), "Hello, World!");

    buffer.discard_prefix(7); // Discard "Hello, "

    EXPECT_EQ(buffer.readable_size(), 6);
    readable = buffer.readable();
    EXPECT_EQ(std::string(readable.data(), readable.size()), "World!");

    buffer.discard_prefix(6); // Discard "World!"

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readable_size(), 0);

}