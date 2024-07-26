#pragma once

#include "allocator.h"
#include "maxmin.h"

struct Claim {
    uint32_t blocks_, term_;

    Claim(uint32_t blocks, uint32_t term) : blocks_(blocks), term_(term) {
    }
};

class SharpAllocator : public Allocator {
   public:
    SharpAllocator(uint64_t num_blocks, float OD, uint32_t tau);

    virtual ~SharpAllocator() = default;

    void add_tenant(uint32_t id);

    void remove_tenant(uint32_t id);

    void allocate();

    void set_demand(uint32_t id, uint32_t demand, bool greedy);

    uint32_t get_fair_share();

    uint32_t get_num_tenants();

    uint32_t get_allocation(uint32_t id);

    uint32_t get_tickets(uint32_t id);

    uint64_t get_available_tickets();

   private:
    struct Tenant {
        uint32_t num_tickets_ = 0;
        std::vector<Claim> claims_;
        uint32_t demand_ = 0, allocation_ = 0;

        void grant_claim(Claim claim);

        uint32_t expire_claims();
    };

    void delegate_claims();

    void expire_claims();

    void redeem_claims();

    MaxMinAllocator claim_alloc_;
    uint32_t claim_term_;
    std::unordered_map<uint32_t, Tenant> tenants_;
};