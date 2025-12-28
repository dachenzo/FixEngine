#include <gtest/gtest.h>
#include <fix/core/Validator.hpp>


TEST(TagScratchTests, SetGetClearFull) {
    Fix::TagScratch tagscratch;
    tagscratch.ensure_bits(130); // need at least 3 uint64_t

    // Initially all bits should be clear
    for (std::size_t i = 0; i < 130; i++) {
        EXPECT_FALSE(tagscratch.get(i));
    }
    EXPECT_FALSE(tagscratch.full(130));

    // Set some bits
    tagscratch.set(0);
    tagscratch.set(65);
    tagscratch.set(129);

    EXPECT_TRUE(tagscratch.get(0));
    EXPECT_TRUE(tagscratch.get(65));
    EXPECT_TRUE(tagscratch.get(129));
    EXPECT_FALSE(tagscratch.get(1));
    EXPECT_FALSE(tagscratch.get(64));
    EXPECT_FALSE(tagscratch.get(128));
    EXPECT_FALSE(tagscratch.full(130));

    // Set all bits
    for (std::size_t i = 0; i < 130; i++) {
        tagscratch.set(i);
    }
    EXPECT_TRUE(tagscratch.full(130));

    // Clear and check again
    tagscratch.clear();
    for (std::size_t i = 0; i < 130; i++) {
        EXPECT_FALSE(tagscratch.get(i));
    }
    EXPECT_FALSE(tagscratch.full(130));
}

TEST(TagScratchTests, SetIterator) {
    Fix::TagScratch tagscratch;
    

    Fix::Message::GenericMessage message = {
        {8, "FIX.4.4"},
        {9, "100"},
        {35, "A"},
        {49, "CLIENT1"},
        {56, "SERVER1"},
        {34, "1"},
        {52, "20251224-09:30:00.000"},
        {98, "0"},
        {108,"30"},
        {10, "235"}
    };

    tagscratch.ensure_bits(message.size());

    // Set bits using iterators
    for (auto it = message.begin(); it != message.end(); ++it) {
        tagscratch.set(it, message.begin());
    }

    // Check that all bits are set
    for (std::size_t i = 0; i < message.size(); i++) {
        EXPECT_TRUE(tagscratch.get(i));
    }
    EXPECT_TRUE(tagscratch.full(message.size()));
}


TEST(TagScratchTests, EnsureBitsResizing) {
    Fix::TagScratch tagscratch;

    // Initially ensure a small size
    tagscratch.ensure_bits(64);
    for (std::size_t i = 0; i < 64; i++) {
        EXPECT_FALSE(tagscratch.get(i));
    }

    // Now ensure a larger size
    tagscratch.ensure_bits(200);
    for (std::size_t i = 0; i < 200; i++) {
        EXPECT_FALSE(tagscratch.get(i));
    }

    // Set some bits in the larger range
    tagscratch.set(150);
    EXPECT_TRUE(tagscratch.get(150));
    EXPECT_FALSE(tagscratch.get(149));
}

TEST(TagScrtatchTests, FullCheckEdgeCases) {
    Fix::TagScratch tagscratch;

    // Test with size exactly multiple of 64
    tagscratch.ensure_bits(128);
    for (std::size_t i = 0; i < 128; i++) {
        tagscratch.set(i);
    }
    EXPECT_TRUE(tagscratch.full(128));

    tagscratch.clear();
    EXPECT_FALSE(tagscratch.full(128));

    // Test with size not multiple of 64
    tagscratch.ensure_bits(130);
    for (std::size_t i = 0; i < 130; i++) {
        tagscratch.set(i);
    }
    EXPECT_TRUE(tagscratch.full(130));

    tagscratch.clear();
    EXPECT_FALSE(tagscratch.full(130));

    // test full check with partially set last uint64_t
    tagscratch.ensure_bits(130);
    for (std::size_t i = 0; i < 126; i++) {
        tagscratch.set(i);  
    }


    EXPECT_FALSE(tagscratch.full(130)); 
    EXPECT_TRUE(tagscratch.full(126));    
    tagscratch.set(126);
    tagscratch.set(127);
    tagscratch.set(128);
    tagscratch.set(129);
    EXPECT_TRUE(tagscratch.full(130));
}