#include <fstream>
#include <vector>

#include "allocator/karma.h"
#include "allocator/maxmin.h"
#include "allocator/mpsp.h"
#include "allocator/sharp.h"
#include "allocator/static.h"
#include "simulation.h"
#include "utils.h"

uint32_t valuation(uint32_t q) {
    return 100;
}

void output_sim(Simulation& s, std::ofstream& out, std::string label) {
    s.output_sim(out, label);
    std::cout << label << " ";
    std::cout.flush();
}

int main(int argc, char** argv) {
    if (argc < 4 || argc > 5) {
        std::cerr << "usage: num_blocks num_tenants num_quanta" << std::endl;
        std::cerr << "       num_blocks num_tenants num_quanta demands_filename" << std::endl;
        return 0;
    }

    uint32_t B = std::atoi(argv[1]), N = std::atoi(argv[2]), T = std::atoi(argv[3]);
    uint32_t fair_share = B / N;

    matrix demands;
    if (argc == 4) {
        demands = generate_uniform_demands(N, T, fair_share * 2);
    } else {
        demands = read_demands(argv[4], N, T, false);
    }
    assert(demands.size() == T && demands[0].size() == N);

    std::ofstream sim_out("test/simulator/out/sim.csv");
    for (int sigma = 0; sigma <= 100; sigma += 20) {
        std::cout << "sigma=" << sigma << ": ";
        std::cout.flush();

        StaticAllocator static_alloc(B);
        MaxMinAllocator maxmin_alloc(B);
        KarmaAllocator karma(B, 1, B * T);
        MPSPAllocator mpsp(B, 0, valuation);
        SharpAllocator sharp(B, 2, 2);
        Simulation s(N, T, sigma);

        s.simulate(static_alloc, demands);
        output_sim(s, sim_out, "static");

        s.simulate(maxmin_alloc, demands);
        output_sim(s, sim_out, "maxmin");

        s.simulate(karma, demands);
        output_sim(s, sim_out, "karma");

        s.simulate(mpsp, demands);
        output_sim(s, sim_out, "mpsp");

        s.simulate(sharp, demands);
        output_sim(s, sim_out, "sharp");
        std::cout << std::endl;
    }
    sim_out.close();
}