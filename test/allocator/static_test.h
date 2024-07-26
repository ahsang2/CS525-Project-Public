#include <gtest/gtest.h>

#include "allocator/static.h"

TEST(StaticAllocatorTest, TenantUnderDemand) {
    StaticAllocator alloc(4);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 1, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 2);
    EXPECT_EQ(alloc.get_allocation(2), 2);
}

TEST(StaticAllocatorTest, UnevenFullDemand) {
    StaticAllocator alloc(4);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 3, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 2);
    EXPECT_EQ(alloc.get_allocation(2), 2);
}

TEST(StaticAllocatorTest, OverDemand) {
    StaticAllocator alloc(4);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 2, false);
    alloc.set_demand(2, 3, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 2);
    EXPECT_EQ(alloc.get_allocation(2), 2);
}
