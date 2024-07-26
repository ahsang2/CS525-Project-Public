#include <functional>
#include <vector>

#include "allocator/karma.h"
#include "allocator/mpsp.h"
#include "allocator/sharp.h"

typedef std::vector<std::vector<uint32_t>> matrix;

struct Simulation {
    uint32_t N_, T_;
    int sigma_;

    std::vector<double> welfares_, instant_fairness_, proxy_;

    double utilization_ = 0, avg_fairness_ = 0, fairness_ = 0;
    double avg_welfare_ = 0, incentive_ = 0;
    double proxy_alt_ = 0, proxy_selfish_ = 0;

    Simulation(uint32_t N, uint32_t T, int sigma);

    void simulate(Allocator& alloc, matrix& demands);

    void simulate(KarmaAllocator& alloc, matrix& demands);

    void simulate(MPSPAllocator& alloc, matrix& demands);

    void simulate(SharpAllocator& alloc, matrix& demands);

    void output_sim(std::ostream& out, std::string label);
};