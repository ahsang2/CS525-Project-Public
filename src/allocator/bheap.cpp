#include "allocator/bheap.h"

BroadcastHeap::BroadcastHeap() {
}

void BroadcastHeap::push(uint32_t key, int32_t val) {
    h_.push(std::make_pair(key, val - base_val_));
}

bheap_item BroadcastHeap::pop() {
    auto i = h_.top();
    h_.pop();

    return std::make_pair(i.first, i.second + base_val_);
}

int32_t BroadcastHeap::min() {
    return h_.top().second + base_val_;
}

void BroadcastHeap::add_all(int32_t delta) {
    base_val_ += delta;
}

size_t BroadcastHeap::size() {
    return h_.size();
}

bool BroadcastHeap::empty() {
    return size() == 0;
}
