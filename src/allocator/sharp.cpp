#include "allocator/sharp.h"

#include "utils.h"

void SharpAllocator::Tenant::grant_claim(Claim claim) {
    num_tickets_ += claim.blocks_;
    claims_.push_back(claim);
}

uint32_t SharpAllocator::Tenant::expire_claims() {
    uint32_t expired_blocks = 0;
    uint32_t alloc = allocation_;
    for (auto it = claims_.begin(); it != claims_.end();) {
        if (alloc) {
            uint32_t redeemed = std::min(alloc, it->blocks_);
            it->blocks_ -= redeemed;
            alloc -= redeemed;
        }

        if (it->blocks_ && --it->term_) {
            ++it;
        } else {
            expired_blocks += it->blocks_;
            it = claims_.erase(it);
        }
    }

    uint32_t lost_tickets = allocation_ + expired_blocks;
    num_tickets_ -= lost_tickets;
    return lost_tickets;
}

SharpAllocator::SharpAllocator(uint64_t num_blocks, float OD, uint32_t claim_term)
    : Allocator(num_blocks), claim_alloc_(num_blocks), claim_term_(claim_term) {
    if (OD < 1) {
        std::cout << "warning: oversubscription degree less than 1" << std::endl;
    }
    claim_alloc_.set_num_blocks(OD * num_blocks);
}

void SharpAllocator::add_tenant(uint32_t id) {
    if (id == PUBLIC_ID || tenants_.find(id) != tenants_.end()) {
        throw std::out_of_range("add_tenant(): tenant ID already exists");
    }
    tenants_[id] = Tenant();
    claim_alloc_.add_tenant(id);
}

void SharpAllocator::remove_tenant(uint32_t id) {
    if (id == PUBLIC_ID || tenants_.find(id) == tenants_.end()) {
        throw std::out_of_range("remove_tenant(): tenant ID does not exist");
    }
    tenants_.erase(id);
    claim_alloc_.remove_tenant(id);
}

void SharpAllocator::allocate() {
    delegate_claims();
    redeem_claims();
    expire_claims();
}

void SharpAllocator::set_demand(uint32_t id, uint32_t demand, bool greedy) {
    auto it = tenants_.find(id);
    if (id == PUBLIC_ID || it == tenants_.end()) {
        throw std::out_of_range("set_demand(): tenant ID does not exist");
    }

    if (greedy) {
        demand = std::max(get_fair_share(), demand);
    }
    it->second.demand_ = demand;
    claim_alloc_.set_demand(id, demand, greedy);
}

void SharpAllocator::delegate_claims() {
    claim_alloc_.allocate();

    uint64_t total_tickets = 0;
    for (auto& [id, t] : tenants_) {
        uint32_t tickets = claim_alloc_.get_allocation(id);
        t.grant_claim(Claim(tickets, claim_term_));
        total_tickets += tickets;
    }
    claim_alloc_.add_num_blocks(-total_tickets);
}

void SharpAllocator::redeem_claims() {
    uint64_t total_redeeem = 0;
    for (auto& [_, t] : tenants_) {
        total_redeeem += std::min(t.demand_, t.num_tickets_);
    }

    if (total_redeeem <= num_blocks_) {
        for (auto& [_, t] : tenants_) {
            t.allocation_ = std::min(t.demand_, t.num_tickets_);
        }
    } else {
        std::vector<uint32_t> weights(get_num_tenants(), 0);
        // Assume tenant IDs are in [1, N]
        for (auto& [id, t] : tenants_) {
            t.allocation_ = 0;
            if (t.demand_ > 0) {
                weights[id - 1] = t.num_tickets_;
            }
        }
        auto dist = get_rand_discrete(weights);

        for (uint32_t i = 0; i < num_blocks_; ++i) {
            uint32_t id = sample_rand_discrete(dist) + 1;
            auto& t = tenants_[id];

            assert(t.allocation_ < t.demand_ && t.allocation_ < t.num_tickets_);
            if (++t.allocation_ == std::min(t.demand_, t.num_tickets_)) {
                weights[id - 1] = 0;
                dist = get_rand_discrete(weights);
            }
        }
    }
}

void SharpAllocator::expire_claims() {
    uint64_t recovered_tickets = 0;
    for (auto& [_, t] : tenants_) {
        recovered_tickets += t.expire_claims();
    }
    claim_alloc_.add_num_blocks(recovered_tickets);
}

uint32_t SharpAllocator::get_fair_share() {
    return get_available_tickets() / get_num_tenants();
}

uint32_t SharpAllocator::get_num_tenants() {
    return tenants_.size();
}

uint32_t SharpAllocator::get_allocation(uint32_t id) {
    auto it = tenants_.find(id);
    if (it == tenants_.end()) {
        throw std::out_of_range("get_allocation(): tenant ID does not exist");
    }
    return it->second.allocation_;
}

uint32_t SharpAllocator::get_tickets(uint32_t id) {
    auto it = tenants_.find(id);
    if (it == tenants_.end()) {
        throw std::out_of_range("get_allocation(): tenant ID does not exist");
    }
    return it->second.num_tickets_;
}

uint64_t SharpAllocator::get_available_tickets() {
    return claim_alloc_.get_num_blocks();
}