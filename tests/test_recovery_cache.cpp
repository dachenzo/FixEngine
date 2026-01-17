#include <gtest/gtest.h>
#include <fix/core/RecoveryCache.hpp>



using namespace Fix;
TEST(RecoveryCacheTest, InsertAndRetrieveMessage) {
    RecoveryCache cache;
    cache.start(1);

    std::string_view msg1 = "8=FIX.4.2|9=12|35=A|10=123|";
    std::string_view msg2 = "8=FIX.4.2|9=14|35=B|10=456|";

    cache.insert(1, msg1);
    cache.insert(2, msg2);

    EXPECT_TRUE(cache.contains(1));
    EXPECT_TRUE(cache.contains(2));

    EXPECT_EQ(cache.get(1), msg1);
    EXPECT_EQ(cache.get(2), msg2);
}


TEST(RecoveryCacheTest, ConsumeMessage) {
    RecoveryCache cache;
    cache.start(1);

    std::string_view msg1 = "8=FIX.4.2|9=12|35=A|10=123|";
    cache.insert(1, msg1);

    EXPECT_TRUE(cache.contains(1));
    cache.consume(1);
    EXPECT_FALSE(cache.contains(1));
}


TEST(RecoveryCacheTest, WindowBoundary) {
    RecoveryCache cache;
    cache.start(1000);

    std::string_view msg = "8=FIX.4.2|9=12|35=A|10=123|";

    // Insert messages within the window
    for (uint64_t i = 1000; i < 1000 + RecoveryCache::window; ++i) {
        cache.insert(i, msg);
        EXPECT_TRUE(cache.contains(i));
    }

    // Attempt to insert message outside the window
    EXPECT_DEATH(cache.insert(1000 + RecoveryCache::window, msg), "SeqNum out of window");
}


TEST(RecoveryCacheTest, ResetCache) {
    RecoveryCache cache;
    cache.start(1);

    std::string_view msg1 = "8=FIX.4.2|9=12|35=A|10=123|";
    cache.insert(1, msg1);

    EXPECT_TRUE(cache.contains(1));

    cache.start(10); // Reset and start at a new sequence number

    EXPECT_FALSE(cache.contains(1)); // Old messages should be gone
    EXPECT_FALSE(cache.contains(10)); // New message not yet inserted

    std::string_view msg2 = "8=FIX.4.2|9=14|35=B|10=456|";
    cache.insert(10, msg2);
    EXPECT_TRUE(cache.contains(10));
    EXPECT_EQ(cache.get(10), msg2);
}