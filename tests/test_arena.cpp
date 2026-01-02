#include <gtest/gtest.h>
#include <fix/core/Arena.hpp>


TEST(ArenaTest, Allocate) {
    Fix::Arena arena;
    

    auto handle1 = arena.allocate(Fix::Arena::block_size);
    EXPECT_TRUE(handle1);
    EXPECT_EQ(handle1.size(), Fix::Arena::block_size);
    EXPECT_NE(handle1.data(), nullptr);

    auto handle2 = arena.allocate(Fix::Arena::block_size+1); // Exceeds block size
    EXPECT_FALSE(handle2);
    EXPECT_EQ(handle2.size(), 0);
    EXPECT_EQ(handle2.data(), nullptr);

}

TEST(ArenaTest, MultipleAllocations) {
    Fix::Arena arena;
    std::vector<Fix::ArenaHandle> handles;

    for (size_t i = 0; i < Fix::Arena::block_count; ++i) {
        auto handle = arena.allocate(Fix::Arena::block_size);
        EXPECT_TRUE(handle);
        handles.push_back(std::move(handle));
    }

    // Next allocation should fail as all blocks are used
    auto handle = arena.allocate(Fix::Arena::block_size);
    EXPECT_FALSE(handle);
}


TEST(ArenaTest, ReleaseOnDestruction) {
    Fix::Arena arena;
    std::vector<Fix::ArenaHandle> handles;

    for (size_t i = 0; i < Fix::Arena::block_count-1; ++i) {
        auto handle = arena.allocate(Fix::Arena::block_size);
        EXPECT_TRUE(handle);
        handles.push_back(std::move(handle));
    }

    {
        auto handle = arena.allocate(Fix::Arena::block_size);
        EXPECT_TRUE(handle);
        auto handle2 = arena.allocate(Fix::Arena::block_size);
        EXPECT_FALSE(handle2); // No more blocks available
    } // handle goes out of scope and should release the block

    // Now we should be able to allocate again
    auto handle3 = arena.allocate(Fix::Arena::block_size);
    EXPECT_TRUE(handle3);
}