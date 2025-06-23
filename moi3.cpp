#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <chrono>
#include <tuple>
#include <string>
#include <fstream>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Item {
    int peso;
    int valor;
};

class CuckooSearchKnapsack {
private:
    std::vector<Item> items;
    int capacity;
    int n_nests;
    int max_iter;
    double pa;
    double alfa;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
    std::uniform_int_distribution<> binary_dis;
    std::normal_distribution<> normal_dis;

public:
    CuckooSearchKnapsack(const std::vector<Item>& items, int cap, int nests, int iterations, double prob_abandon, double a)
        : items(items), capacity(cap), n_nests(nests), max_iter(iterations), pa(prob_abandon), alfa(a),
          gen(std::random_device{}()), dis(0.0, 1.0), binary_dis(0, 1), normal_dis(0.0, 1.0) {}

    int fitness(const std::vector<int>& solution) {
        int total_weight = 0, total_value = 0;
        for (size_t i = 0; i < solution.size(); ++i) {
            if (solution[i] == 1) {
                total_weight += items[i].peso;
                total_value += items[i].valor;
            }
        }
        return (total_weight > capacity) ? 0 : total_value;
    }

    std::vector<int> generateRandomSolution() {
        std::vector<int> nest(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            nest[i] = binary_dis(gen);
        }
        return nest;
    }

    std::vector<int> levyFlight(const std::vector<int>& current_solution) {
        double beta = 1.5;
        double sigma = std::pow(std::tgamma(1 + beta) * std::sin(M_PI * beta / 2) /
                                 std::tgamma((1 + beta) / 2) * std::pow(M_PI, 0.5), 1.0 / beta);
        std::vector<int> new_solution(current_solution.size());
        for (size_t i = 0; i < current_solution.size(); ++i) {
            double levy = normal_dis(gen) * sigma;
            double new_value = current_solution[i] + alfa * levy;
            double sigmoide = 1.0 / (1.0 + std::exp(-std::abs(new_value)));
            double r = dis(gen);
            new_solution[i] = (r < sigmoide) ? 1 : 0;
        }
        return new_solution;
    }

    std::tuple<int, double, double> repeatedRuns(int repetitions) {
        std::vector<double> durations;
        int best_value = 0;

        for (int rep = 0; rep < repetitions; ++rep) {
            auto start = std::chrono::high_resolution_clock::now();

            std::vector<std::vector<int>> nests(n_nests);
            std::vector<int> fitness_values(n_nests);
            for (int i = 0; i < n_nests; ++i) {
                nests[i] = generateRandomSolution();
                fitness_values[i] = fitness(nests[i]);
            }

            int best_idx = std::max_element(fitness_values.begin(), fitness_values.end()) - fitness_values.begin();
            std::vector<int> best_nest = nests[best_idx];
            int best_fitness = fitness_values[best_idx];

            int iteration = 0;
            while (iteration < max_iter) {
                std::uniform_int_distribution<> nest_dis(0, n_nests - 1);
                int i = nest_dis(gen);
                std::vector<int> new_nest = levyFlight(nests[i]);
                int new_fitness = fitness(new_nest);
                int j;
                do {
                    j = nest_dis(gen);
                } while (j == i);

                if (new_fitness > fitness_values[j]) {
                    nests[j] = new_nest;
                    fitness_values[j] = new_fitness;
                }

                int new_best_idx = std::max_element(fitness_values.begin(), fitness_values.end()) - fitness_values.begin();
                int new_best_fitness = fitness_values[new_best_idx];
                if (new_best_fitness > best_fitness) {
                    best_fitness = new_best_fitness;
                    best_nest = nests[new_best_idx];
                }

                int num_replacements = static_cast<int>(pa * n_nests);
                std::vector<int> indices(n_nests);
                std::iota(indices.begin(), indices.end(), 0);
                std::sort(indices.begin(), indices.end(), [&](int a, int b) { return fitness_values[a] < fitness_values[b]; });
                for (int k = 0; k < num_replacements; ++k) {
                    int idx = indices[k];
                    nests[idx] = generateRandomSolution();
                    fitness_values[idx] = fitness(nests[idx]);
                }

                iteration++;
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            durations.push_back(duration.count());
            if (best_fitness > best_value) best_value = best_fitness;
        }

        double avg = std::accumulate(durations.begin(), durations.end(), 0.0) / durations.size();
        double sq_sum = std::inner_product(durations.begin(), durations.end(), durations.begin(), 0.0);
        double std_dev = std::sqrt(sq_sum / durations.size() - avg * avg);

        return {best_value, avg, std_dev};
    }
};

int main() {
    std::ifstream config_file("config.txt");
    std::ofstream out("resultados.csv");
    out << "n_items,n_nests,max_iter,best_fitness,avg_time,std_dev\n";

    std::string line;
    while (std::getline(config_file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        int n_items, n_nests, max_iter, repetitions;
        iss >> n_items >> n_nests >> max_iter >> repetitions;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> peso_dist(1, 10);
        std::uniform_int_distribution<> valor_dist(10, 100);

        std::vector<Item> items(n_items);
        int total_peso = 0;
        for (int i = 0; i < n_items; ++i) {
            items[i].peso = peso_dist(gen);
            items[i].valor = valor_dist(gen);
            total_peso += items[i].peso;
        }

        int capacity = static_cast<int>(0.4 * total_peso);
        double pa = 0.25;
        double a = 1.0;

        CuckooSearchKnapsack cuckoo(items, capacity, n_nests, max_iter, pa, a);
        auto [best_fitness, avg_time, std_dev] = cuckoo.repeatedRuns(repetitions);

        out << n_items << "," << n_nests << "," << max_iter << ","
            << best_fitness << "," << avg_time << "," << std_dev << "\n";
    }

    out.close();
    return 0;
}
