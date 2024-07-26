#include "utils.h"

#include <assert.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

std::random_device rd;
std::mt19937 gen(rd());

bool rand_bool() {
    static auto dist = std::uniform_int_distribution(0, 1);
    return dist(gen);
}

int rand_uniform(int min, int max) {
    auto dist = std::uniform_int_distribution(min, max);
    return dist(gen);
}

std::discrete_distribution<> get_rand_discrete(std::vector<uint32_t>& weights) {
    return std::discrete_distribution<>(weights.begin(), weights.end());
}

int sample_rand_discrete(std::discrete_distribution<>& dist) {
    return dist(gen);
}

matrix generate_uniform_demands(uint32_t N, uint32_t T, uint32_t max_demand) {
    matrix demands(T, std::vector<uint32_t>(N));

    for (uint32_t t = 0; t < T; ++t) {
        for (uint32_t i = 0; i < N; ++i) {
            demands[t][i] = rand_uniform(0, (int)max_demand);
        }
    }
    return demands;
}

matrix read_demands(char* filename, uint32_t N, uint32_t T, bool shuffle) {
    matrix demands(T, std::vector<uint32_t>(N));

    std::ifstream file(filename);
    if (!file) {
        throw std::ios_base::failure("failed to open demands file");
    }

    for (uint32_t t = 0; t < T; ++t) {
        for (uint32_t i = 0; i < N; ++i) {
            file >> demands[t][i];
        }
    }
    file.close();

    if (shuffle) {
        std::shuffle(demands.begin(), demands.end(), gen);
    }
    return demands;
}

std::vector<double> welfares(matrix& demands, matrix& allocations) {
    uint32_t N = demands[0].size();
    std::vector<double> welfares(N);

    for (uint32_t i = 0; i < N; ++i) {
        uint64_t used = 0, total_demand = 0;
        for (uint32_t t = 0; t < demands.size(); ++t) {
            if (demands[t][i] > 0) {
                used += std::min(demands[t][i], allocations[t][i]);
                total_demand += demands[t][i];
            }
        }
        welfares[i] = total_demand > 0 ? (double)used / total_demand : 1;
    }
    return welfares;
}

std::vector<double> welfares(matrix& demands, matrix& allocations,
                             matrix& payments, fi valuation) {
    uint32_t N = demands[0].size();
    std::vector<double> welfares(N);

    for (uint32_t i = 0; i < N; ++i) {
        double actual = 0, expected = 0;
        for (uint32_t t = 0; t < demands.size(); ++t) {
            if (demands[t][i] > 0) {
                double w = (double)std::min(demands[t][t], allocations[t][i]) * valuation(demands[t][i]) / payments[t][i];
                actual += std::min((double)demands[t][i], w);
                expected += demands[t][i];
            }
        }
        welfares[i] = expected > 0 ? actual / expected : 1;
    }
    return welfares;
}

double fairness(std::vector<double>& welfares, size_t si) {
    auto minmax = std::minmax_element(welfares.begin() + si, welfares.end());
    return *minmax.second > 0 ? *minmax.first / *minmax.second : 1;
}

double instant_fairness(std::vector<uint32_t>& demands, std::vector<uint32_t>& allocations, size_t si) {
    double min_welfare = 1, max_welfare = 0;
    for (uint32_t i = si; i < demands.size(); ++i) {
        double welfare = 1;
        if (demands[i] > 0) {
            welfare = (double)std::min(demands[i], allocations[i]) / demands[i];
        }

        min_welfare = std::min(min_welfare, welfare);
        max_welfare = std::max(max_welfare, welfare);
    }
    return max_welfare > 0 ? min_welfare / max_welfare : 1;
}

double instant_fairness(std::vector<uint32_t>& demands, std::vector<uint32_t>& allocations,
                        std::vector<uint32_t>& payments, fi valuation, size_t si) {
    double min_welfare = 1, max_welfare = 0;
    for (uint32_t i = si; i < demands.size(); ++i) {
        double welfare = 1;
        if (demands[i] > 0) {
            double val = (double)std::min(demands[i], allocations[i]) * valuation(demands[i]) / payments[i];
            welfare = std::min((double)demands[i], val) / demands[i];
        }

        min_welfare = std::min(min_welfare, welfare);
        max_welfare = std::max(max_welfare, welfare);
    }
    return max_welfare > 0 ? min_welfare / max_welfare : 1;
}

double utilization(matrix& demands, matrix& allocations, uint64_t blocks) {
    assert(blocks > 0);
    uint32_t T = demands.size();

    uint64_t used = 0;
    for (uint32_t t = 0; t < T; ++t) {
        for (uint32_t i = 0; i < demands[t].size(); ++i) {
            used += std::min(demands[t][i], allocations[t][i]);
        }
    }
    return (double)used / (blocks * T);
}

double range_average(std::vector<double>& arr, size_t a, size_t b) {
    assert(b >= a);

    double sum = 0;
    for (size_t i = a; i < b; ++i) {
        sum += arr[i];
    }
    return b > a ? sum / (b - a) : 0;
}

void clamp(double* a, double* b) {
    if (*a == 0) {
        *a = *b;
    } else if (*b == 0) {
        *b = *a;
    }
}