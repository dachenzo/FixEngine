#include <gtest/gtest.h>
#include <fix/core/Ring.hpp>



TEST(Ring, BasicPushPopGrowPolicy) {
    Fix::Ring<int, 4, 1, Fix::RingPolicy::Grow> ring;

    EXPECT_TRUE(ring.empty());

    EXPECT_TRUE(ring.push(1));
    EXPECT_TRUE(ring.push(2));
    EXPECT_TRUE(ring.push(3));
    EXPECT_TRUE(ring.push(4));

    EXPECT_FALSE(ring.empty());
    EXPECT_TRUE(ring.full());

    EXPECT_TRUE(ring.push(5)); // should grow

    EXPECT_EQ(ring.size(), 5);
    EXPECT_EQ(ring.front(), 1);
    EXPECT_EQ(ring.back(), 5);

    ring.pop_front();
    EXPECT_EQ(ring.front(), 2);
    ring.pop_back();
    EXPECT_EQ(ring.back(), 4);
    EXPECT_EQ(ring.size(), 3);
    ring.pop_front();
    EXPECT_EQ(ring.front(), 3);
    ring.pop_front();
    ring.pop_front();
    
    EXPECT_TRUE(ring.empty());
}

TEST(Ring, PushRejectPolicy) {
    Fix::Ring<int, 3, 0, Fix::RingPolicy::Reject> ring;

    EXPECT_TRUE(ring.push(1)); //1
    EXPECT_TRUE(ring.push(2)); //12
    EXPECT_TRUE(ring.push(3)); //123

    EXPECT_FALSE(ring.push(4)); // should be rejected

    EXPECT_EQ(ring.size(), 3);
    EXPECT_EQ(ring.front(), 1);
    EXPECT_EQ(ring.back(), 3);

    ring.pop_front(); //23
    EXPECT_EQ(ring.front(), 2);
    EXPECT_TRUE(ring.push(4)); // should succeed now //234
    EXPECT_EQ(ring.back(), 4);
    ring.pop_back(); //23
    EXPECT_EQ(ring.back(), 3);
    EXPECT_EQ(ring.size(), 2);
    ring.pop_front(); //3
    ring.pop_front(); //empty
    
    EXPECT_TRUE(ring.empty());
}


TEST(Ring, PushOverwritePolicy) {
    Fix::Ring<int, 3, 0, Fix::RingPolicy::Overwrite> ring;

    EXPECT_TRUE(ring.push(1)); //1
    EXPECT_TRUE(ring.push(2)); //12
    EXPECT_TRUE(ring.push(3)); //123

    EXPECT_TRUE(ring.push(4)); // should overwrite 1 -> 234
    EXPECT_EQ(ring.size(), 3);
    EXPECT_EQ(ring.front(), 2);
    EXPECT_EQ(ring.back(), 4);

    ring.pop_front(); //34
    EXPECT_EQ(ring.front(), 3);
    EXPECT_TRUE(ring.push(5)); // should succeed now -> 345
    EXPECT_EQ(ring.back(), 5);
    ring.pop_back(); //34
    EXPECT_EQ(ring.back(), 4);
    EXPECT_EQ(ring.size(), 2);
    ring.pop_front(); //4
    ring.pop_front(); //empty
    
    EXPECT_TRUE(ring.empty());
}