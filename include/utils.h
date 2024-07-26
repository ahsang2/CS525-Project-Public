#include <random>
#include <vector>

#include "types.h"

bool rand_bool();

int rand_uniform(int min, int max);

std::discrete_distribution<> get_rand_discrete(std::vector<uint32_t>& weights);

int sample_rand_discrete(std::discrete_distribution<>& dist);

matrix generate_uniform_demands(uint32_t N, uint32_t T, uint32_t max_demand);

matrix read_demands(char* filename, uint32_t N, uint32_t T, bool shuffle);

std::vector<double> welfares(matrix& demands, matrix& allocations);

std::vector<double> welfares(matrix& demands, matrix& allocations,
                             matrix& payments, fi valuation);

double fairness(std::vector<double>& welfares, size_t si);

double instant_fairness(std::vector<uint32_t>& demands, std::vector<uint32_t>& allocations, size_t si);

double instant_fairness(std::vector<uint32_t>& demands, std::vector<uint32_t>& allocations,
                        std::vector<uint32_t>& payments, fi valuation, size_t si);

double utilization(matrix& demands, matrix& allocations, uint64_t blocks);

double range_average(std::vector<double>& arr, size_t a, size_t b);

void clamp(double* a, double* b);