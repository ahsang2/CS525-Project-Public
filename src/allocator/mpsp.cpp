#include "allocator/mpsp.h"

#include <assert.h>

#include <queue>

#include "utils.h"

bool bid_cmp(const pi& a, const pi& b) {
    return a.second < b.second;
}

void MPSPAllocator::Tenant::bid_auction(uint32_t demand, uint32_t fair_share,
                                        bool greedy, pi border_bids) {
    if (demand <= fair_share) {
        bid_.qty_ = 0;
    } else {
        uint32_t qty = demand - fair_share;
        uint32_t val = valuation_(qty);
        assert(val > 0);

        if (greedy) {
            uint32_t delta = 10;
            val += rand_uniform(-delta, delta);
        }
        bid_ = Bid(qty, val);
    }
}

MPSPAllocator::MPSPAllocator(uint64_t num_blocks, uint64_t base_blocks, fi valuation)
    : Allocator(num_blocks), base_blocks_(base_blocks), valuation_(valuation) {
    if (base_blocks > num_blocks) {
        throw std::invalid_argument("number of base blocks must be <= total blocks");
    }
    border_bids_.first = valuation(1);
    border_bids_.second = border_bids_.first;

    tenants_[PUBLIC_ID].bid_ = Bid(get_free_blocks() + 1, border_bids_.first / 2);
}

void MPSPAllocator::add_tenant(uint32_t id) {
    if (id == PUBLIC_ID || tenants_.find(id) != tenants_.end()) {
        throw std::out_of_range("add_tenant(): tenant ID already exists");
    }
    tenants_.emplace(id, valuation_);
}

void MPSPAllocator::remove_tenant(uint32_t id) {
    if (id == PUBLIC_ID || tenants_.find(id) == tenants_.end()) {
        throw std::out_of_range("remove_tenant(): tenant ID does not exist");
    }
    tenants_.erase(id);
}

void MPSPAllocator::allocate() {
    uint32_t fair_share = get_fair_share();
    uint32_t free_blocks = get_free_blocks();
    std::vector<pi> lowest_bids;

    for (auto& [id, t] : tenants_) {
        t.payment_ = 0;
        if (id != PUBLIC_ID) {
            t.allocation_ = fair_share;
        } else {
            t.bid_.qty_ = free_blocks + 1;
        }

        if (t.bid_.qty_ > 0) {
            lowest_bids.emplace_back(id, t.bid_.price_);
        }
    }

    std::sort(lowest_bids.begin(), lowest_bids.end(), bid_cmp);

    uint64_t welfare = 0;
    // std::vector<uint32_t> winners;
    while (free_blocks > 0) {
        auto [id, price] = lowest_bids.back();
        auto& t = tenants_[id];
        assert(t.payment_ == 0);

        uint32_t blocks = std::min(t.bid_.qty_, free_blocks);
        assert(blocks > 0);

        t.bid_.qty_ -= blocks;
        t.allocation_ = blocks;
        t.payment_ = price;

        free_blocks -= blocks;
        welfare += blocks * price;
        border_bids_.first = price;

        if (t.bid_.qty_ == 0) {
            lowest_bids.pop_back();
        }
        // winners.push_back(id);
    }

    assert(!lowest_bids.empty());
    border_bids_.second = lowest_bids.back().second;
    assert(border_bids_.first >= border_bids_.second);

    for (auto& [id, t] : tenants_) {
        if (id != PUBLIC_ID) {
            charge_exclusion_payment(id, lowest_bids, welfare);
        }
    }
}

void MPSPAllocator::charge_exclusion_payment(int id, std::vector<pi>& remaining_bids, uint64_t welfare) {
    auto& t = tenants_[id];
    if (t.payment_ > 0) {
        uint32_t free_blocks = t.allocation_;

        uint64_t exclude_welfare = welfare - t.payment_ * t.allocation_;
        welfare = exclude_welfare;

        for (int i = remaining_bids.size() - 1; i >= 0 && free_blocks > 0; --i) {
            auto [bidder_id, price] = remaining_bids[i];

            if (id != bidder_id) {
                uint32_t blocks = std::min(tenants_[bidder_id].bid_.qty_, free_blocks);
                free_blocks -= blocks;
                welfare += blocks * price;
            }
        }

        uint32_t payment = (welfare - exclude_welfare) / t.allocation_;
        assert(payment > 0 && payment <= t.payment_);
        t.payment_ = payment;
    }
}

void MPSPAllocator::set_demand(uint32_t id, uint32_t demand, bool greedy) {
    auto it = tenants_.find(id);
    if (id == PUBLIC_ID || it == tenants_.end()) {
        throw std::out_of_range("set_demand(): tenant ID does not exist");
    }
    it->second.bid_auction(demand, get_fair_share(), greedy, border_bids_);
}

uint32_t MPSPAllocator::get_num_tenants() {
    return tenants_.size() - 1;
}

uint32_t MPSPAllocator::get_allocation(uint32_t id) {
    auto it = tenants_.find(id);
    if (it == tenants_.end()) {
        throw std::out_of_range("get_allocation(): tenant ID does not exist");
    }
    return it->second.allocation_;
}

uint32_t MPSPAllocator::get_payment(uint32_t id) {
    auto it = tenants_.find(id);
    if (it == tenants_.end()) {
        throw std::out_of_range("get_payment(): tenant ID does not exist");
    }
    return it->second.payment_;
}

fi MPSPAllocator::get_valuation() {
    return valuation_;
}

pi MPSPAllocator::get_border_bids() {
    return border_bids_;
}

uint32_t MPSPAllocator::get_fair_share() {
    return base_blocks_ / get_num_tenants();
}

uint64_t MPSPAllocator::get_free_blocks() {
    return num_blocks_ - base_blocks_;
}