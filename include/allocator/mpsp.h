#pragma once

#include "allocator.h"
#include "types.h"

struct Bid {
    uint32_t qty_ = 0, price_ = 0;

    Bid() {
    }

    Bid(uint32_t qty, uint32_t price) : qty_(qty), price_(price) {
        if (qty < 0 || price < 0) {
            throw std::invalid_argument("bid qty and price must be non-negative");
        }
    }
};

class MPSPAllocator : public Allocator {
   public:
    MPSPAllocator(uint64_t num_blocks, uint64_t base_blocks, fi valuation);

    virtual ~MPSPAllocator() = default;

    void add_tenant(uint32_t id);

    void remove_tenant(uint32_t id);

    void allocate();

    void set_demand(uint32_t id, uint32_t demand, bool greedy);

    uint32_t get_fair_share();

    uint32_t get_num_tenants();

    uint32_t get_allocation(uint32_t id);

    uint32_t get_payment(uint32_t id);

    fi get_valuation();

    pi get_border_bids();

   private:
    struct Tenant {
        Bid bid_;
        uint32_t allocation_ = 0, payment_ = 0;
        fi valuation_;

        Tenant() {
        }

        Tenant(fi valuation) : valuation_(valuation) {
        }

        void bid_auction(uint32_t demand, uint32_t fair_share, bool greedy, pi border_bids);
    };

    uint64_t base_blocks_;
    pi border_bids_;
    fi valuation_;
    std::unordered_map<uint32_t, Tenant> tenants_;

    uint64_t get_free_blocks();

    void charge_exclusion_payment(int id, std::vector<pi>& remaining_bids, uint64_t welfare);
};