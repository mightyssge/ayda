#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <chrono>
#include <tuple>

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

    // Generar una solución mediante Levy flights
    std::vector<int> levyFlight(const std::vector<int>& current_solution) {
        // Formular Levy Flight
        double beta = 1.5;
        double numerator = std::tgamma(1.0 + beta) * std::sin(M_PI * beta / 2.0);
        double denominator = std::tgamma((1.0 + beta) / 2.0) * beta * std::pow(2.0, (beta - 1.0) / 2.0);
        double sigma = std::pow(numerator / denominator, 1.0 / beta);
        
        // Construir nueva solucion
        std::vector<int> new_solution(current_solution.size());
        for (size_t i = 0; i < current_solution.size(); ++i) {
            double u = normal_dis(gen) * sigma;
            double v = normal_dis(gen);
            double levy = u / std::pow(std::abs(v), 1.0 / beta);
            double new_value = current_solution[i] + alfa * levy;
            // ADAPTACION: Convertir la nueva solucion de continua a discreta {0,1}
            double sigmoide = 1.0 / (1.0 + std::exp(-std::abs(new_value)));
            double r = dis(gen);
            new_solution[i] = (r < sigmoide) ? 1 : 0;
        }
        
        return new_solution;
    }

    std::tuple<std::vector<int>, std::vector<int>, std::chrono::duration<double>> cuckooSearch() {
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
        std::vector<int> fitness_evolution;
        fitness_evolution.push_back(best_fitness);

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
            if (fitness_values[new_best_idx] > best_fitness) {
                best_fitness = fitness_values[new_best_idx];
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

            fitness_evolution.push_back(best_fitness);
            iteration++;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        return {best_nest, fitness_evolution, duration};
    }
};

int main() {
    std::vector<Item> items = {
        {7, 70}, {3, 40}, {5, 60}, {8, 80}, {4, 50},
        {6, 55}, {10, 100}, {9, 90}, {2, 30}, {1, 20}
    };
    int capacity = 15;
    int nests = 10;
    int maxGenerations = 1000;
    double pa = 0.25;
    double a = 1.0;

    int count_optimal = 0;
    int count_non_optimal = 0;
    std::vector<double> tiempos;
    std::vector<int> mejores_fitness;

    const int ejecuciones = 10000;
    const int optimo_conocido = 200;

    for (int i = 0; i < ejecuciones; ++i) {
        CuckooSearchKnapsack cuckoo(items, capacity, nests, maxGenerations, pa, a);
        auto [best_solution, fitness_evolution, duration] = cuckoo.cuckooSearch();
        int mejor = fitness_evolution.back();
        mejores_fitness.push_back(mejor);
        tiempos.push_back(duration.count());
        if (mejor == optimo_conocido)
            count_optimal++;
        else
            count_non_optimal++;
    }

    double sum = std::accumulate(tiempos.begin(), tiempos.end(), 0.0);
    double mean = sum / tiempos.size();
    double sq_sum = std::inner_product(tiempos.begin(), tiempos.end(), tiempos.begin(), 0.0);
    double stddev = std::sqrt(sq_sum / tiempos.size() - mean * mean);

    double avg_fitness = std::accumulate(mejores_fitness.begin(), mejores_fitness.end(), 0.0) / mejores_fitness.size();
    double sq_fit = std::inner_product(mejores_fitness.begin(), mejores_fitness.end(), mejores_fitness.begin(), 0.0);
    double stddev_fit = std::sqrt(sq_fit / mejores_fitness.size() - avg_fitness * avg_fitness);

    double error_abs = std::abs(optimo_conocido - avg_fitness);
    double error_rel = (optimo_conocido == 0) ? 0 : (error_abs / optimo_conocido) * 100;

    std::cout << "\n======= RESULTADOS DE " << ejecuciones << " EJECUCIONES =======" << std::endl;
    std::cout << "Tiempo promedio: " << mean << " segundos" << std::endl;
    std::cout << "Desviacion estandar (tiempo): " << stddev << " segundos" << std::endl;
    std::cout << "Fitness promedio: " << avg_fitness << std::endl;
    std::cout << "Desviacion estandar (fitness): " << stddev_fit << std::endl;
    std::cout << "Error absoluto respecto al optimo: " << error_abs << std::endl;
    std::cout << "Error relativo respecto al optimo: " << error_rel << "%" << std::endl;
    std::cout << "Cantidad veces que se alcanzo el optimo (" << optimo_conocido << "): " << count_optimal << "/" << ejecuciones << std::endl;
    std::cout << "Cantidad veces que NO se alcanzo el optimo: " << count_non_optimal << "/" << ejecuciones << std::endl;

    return 0;
}
