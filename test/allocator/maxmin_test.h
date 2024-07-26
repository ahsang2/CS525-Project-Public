#include <gtest/gtest.h>

#include "allocator/maxmin.h"

TEST(MaxMinAllocatorTest, TenantUnderDemand) {
    MaxMinAllocator alloc(4);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 1, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 1);
    EXPECT_EQ(alloc.get_allocation(2), 1);
}

TEST(MaxMinAllocatorTest, UnevenFullDemand) {
    MaxMinAllocator alloc(4);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 3, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 3);
    EXPECT_EQ(alloc.get_allocation(2), 1);
}

TEST(MaxMinAllocatorTest, OverDemand) {
    MaxMinAllocator alloc(6);
    alloc.add_tenant(1);
    alloc.add_tenant(2);
    alloc.add_tenant(3);

    alloc.set_demand(1, 4, false);
    alloc.set_demand(2, 3, false);
    alloc.set_demand(3, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 2);
    EXPECT_EQ(alloc.get_allocation(2), 3);
    EXPECT_EQ(alloc.get_allocation(3), 1);
}
