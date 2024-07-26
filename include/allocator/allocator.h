#pragma once

#include <assert.h>

#include <functional>
#include <iostream>

#define PUBLIC_ID 0

class Allocator {
   public:
    Allocator(uint32_t num_blocks) : num_blocks_(num_blocks) {
    }

    virtual ~Allocator() = default;

    virtual void add_tenant(uint32_t id) = 0;

    virtual void remove_tenant(uint32_t id) = 0;

    virtual void allocate() = 0;

    virtual void set_demand(uint32_t id, uint32_t demand, bool greedy) = 0;

    virtual uint32_t get_fair_share() = 0;

    virtual uint32_t get_num_tenants() = 0;

    virtual uint32_t get_allocation(uint32_t id) = 0;

    void set_num_blocks(uint64_t blocks) {
        num_blocks_ = blocks;
    }

    void add_num_blocks(int blocks) {
        assert(-blocks <= (int)num_blocks_);
        num_blocks_ += blocks;
    }

    uint64_t get_num_blocks() {
        return num_blocks_;
    }

   protected:
    uint64_t num_blocks_;
};