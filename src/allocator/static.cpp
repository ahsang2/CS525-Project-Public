#include "allocator/static.h"

StaticAllocator::StaticAllocator(uint64_t num_blocks) : Allocator(num_blocks) {
}

void StaticAllocator::add_tenant(uint32_t id) {
    if (allocations_.find(id) != allocations_.end()) {
        throw std::out_of_range("add_tenant(): tenant ID already exists");
    }
    allocations_[id] = 0;
}

void StaticAllocator::remove_tenant(uint32_t id) {
    if (allocations_.find(id) == allocations_.end()) {
        throw std::out_of_range("remove_tenant(): tenant ID does not exist");
    }
    allocations_.erase(id);
}

void StaticAllocator::allocate() {
    uint32_t fair_share = get_fair_share();
    for (auto& a : allocations_) {
        a.second = fair_share;
    }
}

void StaticAllocator::set_demand(uint32_t id, uint32_t demand, bool greedy) {
    if (allocations_.find(id) == allocations_.end()) {
        throw std::out_of_range("set_demand(): tenant ID does not exist");
    }
}

uint32_t StaticAllocator::get_fair_share() {
    return num_blocks_ / get_num_tenants();
}

uint32_t StaticAllocator::get_num_tenants() {
    return allocations_.size();
}

uint32_t StaticAllocator::get_allocation(uint32_t id) {
    auto it = allocations_.find(id);
    if (it == allocations_.end()) {
        throw std::out_of_range("get_allocation(): tenant ID does not exist");
    }
    return it->second;
}
