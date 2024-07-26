#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include "allocator.h"

#define DUMMY_ID std::numeric_limits<uint32_t>::max()

class KarmaAllocator : public Allocator {
   public:
    KarmaAllocator(uint64_t num_blocks, float alpha, uint32_t init_credits);

    virtual ~KarmaAllocator() = default;

    void add_tenant(uint32_t id);

    void remove_tenant(uint32_t id);

    void allocate();

    void set_demand(uint32_t id, uint32_t demand, bool greedy);

    uint32_t get_fair_share();

    uint32_t get_num_tenants();

    uint32_t get_allocation(uint32_t id);

    uint32_t get_credits(uint32_t id);

   private:
    struct Tenant {
        uint32_t demand_ = 0, allocation_ = 0, credits_;
        int32_t rate_ = 0;

        Tenant() : credits_(0) {
        }

        Tenant(uint32_t init_credits) : credits_(init_credits) {
        }
    };

    struct Candidate {
        int64_t credits_;
        uint32_t id_, blocks_;

        Candidate(uint32_t id, int64_t credits, uint32_t blocks)
            : id_(id), credits_(credits), blocks_(blocks) {
        }
    };

    uint64_t public_blocks_;
    uint32_t init_credits_;
    std::unordered_map<uint32_t, Tenant> tenants_;

    uint32_t get_block_surplus(uint32_t id);

    uint64_t get_free_blocks();

    void borrow_from_poor(uint64_t demand, std::vector<uint32_t>& donors, std::vector<uint32_t>& borrowers);

    void donate_to_rich(uint64_t supply, std::vector<uint32_t>& donors, std::vector<uint32_t>& borrowers);
};