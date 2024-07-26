#include "simulation.h"

#include "assert.h"
#include "utils.h"

Simulation::Simulation(uint32_t N, uint32_t T, int sigma) : N_(N), T_(T), sigma_(sigma) {
    assert(sigma >= 0 && sigma <= 100);

    welfares_ = std::vector<double>(N);
    instant_fairness_ = std::vector<double>(T);
    proxy_ = std::vector<double>(N, 0);
}

void Simulation::simulate(Allocator& alloc, matrix& demands) {
    size_t si = sigma_ / 100.0 * N_;
    matrix allocations(T_, std::vector<uint32_t>(N_));

    for (uint32_t i = 1; i <= N_; ++i) {
        alloc.add_tenant(i);
    }

    for (uint32_t t = 0; t < T_; ++t) {
        for (uint32_t i = 1; i <= N_; ++i) {
            alloc.set_demand(i, demands[t][i - 1], i <= si);
        }

        alloc.allocate();

        for (uint32_t i = 1; i <= N_; ++i) {
            allocations[t][i - 1] = alloc.get_allocation(i);
        }
        instant_fairness_[t] = instant_fairness(demands[t], allocations[t], si);
    }
    utilization_ = utilization(demands, allocations, alloc.get_num_blocks());
    welfares_ = welfares(demands, allocations);
    fairness_ = fairness(welfares_, si);

    double alt_welfare_ = range_average(welfares_, si, N_);
    double selfish_welfare = range_average(welfares_, 0, si);

    clamp(&alt_welfare_, &selfish_welfare);
    incentive_ = alt_welfare_ - selfish_welfare;

    avg_welfare_ = range_average(welfares_, 0, N_);
    avg_fairness_ = range_average(instant_fairness_, 0, T_);

    proxy_alt_ = 0, proxy_selfish_ = 0;
}

void Simulation::simulate(KarmaAllocator& alloc, matrix& demands) {
    size_t si = sigma_ / 100.0 * N_;
    matrix allocations(T_, std::vector<uint32_t>(N_));

    for (uint32_t i = 1; i <= N_; ++i) {
        alloc.add_tenant(i);
    }

    for (uint32_t t = 0; t < T_; ++t) {
        for (uint32_t i = 1; i <= N_; ++i) {
            alloc.set_demand(i, demands[t][i - 1], i <= si);
        }

        alloc.allocate();

        for (uint32_t i = 1; i <= N_; ++i) {
            allocations[t][i - 1] = alloc.get_allocation(i);
        }
        instant_fairness_[t] = instant_fairness(demands[t], allocations[t], si);
    }
    utilization_ = utilization(demands, allocations, alloc.get_num_blocks());
    welfares_ = welfares(demands, allocations);
    fairness_ = fairness(welfares_, si);

    double alt_welfare_ = range_average(welfares_, si, N_);
    double selfish_welfare = range_average(welfares_, 0, si);

    clamp(&alt_welfare_, &selfish_welfare);
    incentive_ = alt_welfare_ - selfish_welfare;

    avg_welfare_ = range_average(welfares_, 0, N_);
    avg_fairness_ = range_average(instant_fairness_, 0, T_);

    // Karma-specific: get user credits at end of all allocations
    for (uint32_t i = 1; i <= N_; ++i) {
        proxy_[i - 1] = alloc.get_credits(i);
    }
    proxy_alt_ = range_average(proxy_, si, N_);
    proxy_selfish_ = range_average(proxy_, 0, si);
    clamp(&proxy_alt_, &proxy_selfish_);
}

void Simulation::simulate(MPSPAllocator& alloc, matrix& demands) {
    size_t si = sigma_ / 100.0 * N_;
    matrix allocations(T_, std::vector<uint32_t>(N_));
    matrix payments = allocations;

    proxy_ = std::vector<double>(N_, 0);
    std::vector<uint32_t> wins(N_, 0);

    for (uint32_t i = 1; i <= N_; ++i) {
        alloc.add_tenant(i);
    }

    for (uint32_t t = 0; t < T_; ++t) {
        for (uint32_t i = 1; i <= N_; ++i) {
            alloc.set_demand(i, demands[t][i - 1], i <= si);
        }

        alloc.allocate();

        for (uint32_t i = 1; i <= N_; ++i) {
            allocations[t][i - 1] = alloc.get_allocation(i);

            uint32_t payment = alloc.get_payment(i);
            payments[t][i - 1] = payment;
            if (payment > 0) {
                proxy_[i - 1] += payment;
                wins[i - 1]++;
            }
        }
        instant_fairness_[t] = instant_fairness(demands[t], allocations[t], payments[t], alloc.get_valuation(), si);
    }
    utilization_ = utilization(demands, allocations, alloc.get_num_blocks());
    welfares_ = welfares(demands, allocations, payments, alloc.get_valuation());
    fairness_ = fairness(welfares_, si);

    double alt_welfare_ = range_average(welfares_, si, N_);
    double selfish_welfare = range_average(welfares_, 0, si);

    clamp(&alt_welfare_, &selfish_welfare);
    incentive_ = alt_welfare_ - selfish_welfare;

    avg_welfare_ = range_average(welfares_, 0, N_);
    avg_fairness_ = range_average(instant_fairness_, 0, T_);

    // MPSP-specific: get average user winning payments
    for (uint32_t i = 0; i < N_; ++i) {
        proxy_[i] /= wins[i];
    }
    proxy_alt_ = range_average(proxy_, si, N_);
    proxy_selfish_ = range_average(proxy_, 0, si);
    clamp(&proxy_alt_, &proxy_selfish_);
}

void Simulation::simulate(SharpAllocator& alloc, matrix& demands) {
    size_t si = sigma_ / 100.0 * N_;
    matrix allocations(T_, std::vector<uint32_t>(N_));
    proxy_ = std::vector<double>(N_, 0);

    for (uint32_t i = 1; i <= N_; ++i) {
        alloc.add_tenant(i);
    }

    for (uint32_t t = 0; t < T_; ++t) {
        for (uint32_t i = 1; i <= N_; ++i) {
            alloc.set_demand(i, demands[t][i - 1], i <= si);
        }

        alloc.allocate();

        for (uint32_t i = 1; i <= N_; ++i) {
            allocations[t][i - 1] = alloc.get_allocation(i);
            proxy_[i - 1] += alloc.get_tickets(i);
        }
        instant_fairness_[t] = instant_fairness(demands[t], allocations[t], si);
    }
    utilization_ = utilization(demands, allocations, alloc.get_num_blocks());
    welfares_ = welfares(demands, allocations);
    fairness_ = fairness(welfares_, si);

    double alt_welfare = range_average(welfares_, si, N_);
    double selfish_welfare = range_average(welfares_, 0, si);

    clamp(&alt_welfare, &selfish_welfare);
    incentive_ = alt_welfare - selfish_welfare;

    avg_welfare_ = range_average(welfares_, 0, N_);
    avg_fairness_ = range_average(instant_fairness_, 0, T_);

    // Sharp-specific: get average user tickets after each allocation
    for (uint32_t i = 0; i < N_; ++i) {
        proxy_[i] /= T_;
    }
    proxy_alt_ = range_average(proxy_, si, N_);
    proxy_selfish_ = range_average(proxy_, 0, si);
    clamp(&proxy_alt_, &proxy_selfish_);
}

void Simulation::output_sim(std::ostream& out, std::string label) {
    out << label << "," << sigma_ << "," << utilization_ << ","
        << avg_welfare_ << "," << incentive_ << ","
        << fairness_ << "," << avg_fairness_ << ","
        << proxy_alt_ << "," << proxy_selfish_ << std::endl;
}
