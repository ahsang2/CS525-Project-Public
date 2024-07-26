#include <gtest/gtest.h>

#include "allocator/karma.h"

TEST(KarmaAllocatorTest, TenantUnderDemand) {
    KarmaAllocator alloc(4, 0.5, 100);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 1, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 1);
    EXPECT_EQ(alloc.get_allocation(2), 1);
}

TEST(KarmaAllocatorTest, UnevenFullDemand) {
    KarmaAllocator alloc(4, 0.5, 100);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 3, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 3);
    EXPECT_EQ(alloc.get_allocation(2), 1);
}

TEST(KarmaAllocatorTest, OverDemandCreditParity) {
    KarmaAllocator alloc(6, 0.5, 100);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 3, false);
    alloc.set_demand(2, 3, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 2);
    EXPECT_EQ(alloc.get_allocation(2), 3);
}

TEST(KarmaAllocatorTest, OverDemandCreditDisparity) {
    KarmaAllocator alloc(4, 0.5, 100);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 3, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    alloc.set_demand(1, 2, false);
    alloc.set_demand(2, 3, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 1);
    EXPECT_EQ(alloc.get_allocation(2), 3);
}

TEST(KarmaAllocatorTest, GuaranteeFairShareCreditDisparity) {
    KarmaAllocator alloc(4, 0.5, 100);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 0, false);
    alloc.set_demand(2, 4, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 0);
    EXPECT_EQ(alloc.get_allocation(2), 4);

    alloc.set_demand(1, 4, false);
    alloc.set_demand(2, 1, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 3);
    EXPECT_EQ(alloc.get_allocation(2), 1);
}

TEST(KarmaAllocatorTest, GuaranteeFairShareInsufficientCredits) {
    KarmaAllocator alloc(4, 0.5, 0);
    alloc.add_tenant(1);
    alloc.add_tenant(2);

    alloc.set_demand(1, 0, false);
    alloc.set_demand(2, 3, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 0);
    EXPECT_EQ(alloc.get_allocation(2), 2);

    alloc.set_demand(1, 4, false);
    alloc.set_demand(2, 2, false);
    alloc.allocate();

    EXPECT_EQ(alloc.get_allocation(1), 3);
    EXPECT_EQ(alloc.get_allocation(2), 1);
}
