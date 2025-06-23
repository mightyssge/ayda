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

// Estructura para representar un objeto
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
    std::mt19937 gen; // Generador de randoms
    std::uniform_real_distribution<> dis; // Distribucion uniforme de numeros decimales de 0 a 1
    std::uniform_int_distribution<> binary_dis; // Distribucion uniforme de 0 o 1
    std::normal_distribution<> normal_dis; // Distribucion normal

public:
    CuckooSearchKnapsack(
        
        const std::vector<Item>& items, 
        int cap, 
        int nests, 
        int iterations, 
        double prob_abandon,
        double a)
        
        : 
        
        items(items), 
        capacity(cap), 
        n_nests(nests), 
        max_iter(iterations), 
        pa(prob_abandon), 
        alfa(a),
        
        gen(std::random_device{}()), 
        dis(0.0, 1.0), 
        binary_dis(0, 1),
        normal_dis(0.0, 1.0) {}

    // Calcular fitness de una solución (funcion objetivo) 
    int fitness(const std::vector<int>& solution) {
        int total_weight = 0, total_value = 0;
        
        for (size_t i = 0; i < solution.size(); ++i) {
            if (solution[i] == 1) {
                total_weight += items[i].peso;
                total_value += items[i].valor;
            }
        }
        
        // Penalización si excede el peso
        if (total_weight > capacity) {
            return 0;
        }
        
        return total_value;
    }

    // Generar una solucion aleatoria
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
        double sigma = std::pow(std::tgamma(1 + beta) * std::sin(M_PI * beta / 2) / 
                               std::tgamma((1 + beta) / 2) * std::pow(M_PI, 0.5), 1.0/beta);
        
        // Construir nueva solucion
        std::vector<int> new_solution(current_solution.size());
        for (size_t i = 0; i < current_solution.size(); ++i) {
            double levy = normal_dis(gen) * sigma;  
            double new_value = current_solution[i] + alfa * levy;
            // ADAPTACION: Convertir la nueva solucion de continua a discreta {0,1}
            double sigmoide = 1.0 / (1.0 + std::exp(-std::abs(new_value)));
            double r = dis(gen);
            new_solution[i] = (r < sigmoide) ? 1 : 0;
        }
        
        return new_solution;
        
    }

    // Algoritmo Cuckoo Search
    std::tuple<std::vector<int>, std::vector<int>, std::chrono::duration<double>> cuckooSearch() {
        //Auxiliar: Tiempo
        auto start = std::chrono::high_resolution_clock::now();
        
        // Inicializar nidos
        std::vector<std::vector<int>> nests(n_nests);
        std::vector<int> fitness_values(n_nests);
        for (int i = 0; i < n_nests; ++i) {
            nests[i] = generateRandomSolution();
            fitness_values[i] = fitness(nests[i]);
        }

        // Encontrar el mejor nido inicial
        int best_idx = std::max_element(fitness_values.begin(), fitness_values.end()) 
                      - fitness_values.begin();
        std::vector<int> best_nest = nests[best_idx];
        int best_fitness = fitness_values[best_idx];
        // Auxiliar: Evolucion
        std::vector<int> fitness_evolution;
        fitness_evolution.push_back(best_fitness);

        // Iteraciones
        for (int iteration = 0; iteration < max_iter; ++iteration) {
            
            // Obtener nido aleatorio i
            std::uniform_int_distribution<> nest_dis(0, n_nests - 1);
            int i = nest_dis(gen);
            // Generar nueva solución mediante Levy Flight
            std::vector<int> new_nest = levyFlight(nests[i]);
            int new_fitness = fitness(new_nest);

            // Obtener nido aleatorio j
            int j = nest_dis(gen);
            // Comparar con un nido aleatorio
            if (new_fitness > fitness_values[j]) {
                nests[j] = new_nest;
                fitness_values[j] = new_fitness;
            }

            // Reemplazar los peores nidos
            int num_replacements = static_cast<int>(pa * n_nests);
            std::vector<int> indices(n_nests);
            std::iota(indices.begin(), indices.end(), 0);
            // Ordenar índices por fitness (de menor a mayor)
            std::sort(indices.begin(), indices.end(), 
                     [&](int a, int b) { return fitness_values[a] < fitness_values[b]; });
            // Reemplazar los peores
            for (int k = 0; k < num_replacements; ++k) {
                int idx = indices[k];
                nests[idx] = generateRandomSolution();
                fitness_values[idx] = fitness(nests[idx]);
            }

            // Actualizar la mejor solución
            best_idx = std::max_element(fitness_values.begin(), fitness_values.end()) 
                      - fitness_values.begin();
            best_fitness = fitness_values[best_idx];
            best_nest = nests[best_idx];

            fitness_evolution.push_back(best_fitness);
        }
        
        // Auxiliar: tiempo
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        return {best_nest, fitness_evolution, duration};
    }

    // Algoritmo de fuerza bruta para comparación
    std::tuple<std::vector<std::vector<int>>, int, std::chrono::duration<double>> bruteForceSolution() {
        auto start = std::chrono::high_resolution_clock::now();
        int n = items.size();
        int best_value = 0;
        std::vector<std::vector<int>> best_solutions;

        // Generar todas las combinaciones posibles (2^n)
        for (int mask = 1; mask < (1 << n); ++mask) {
            int total_weight = 0, total_value = 0;
            std::vector<int> current_solution(n, 0);

            for (int i = 0; i < n; ++i) {
                if (mask & (1 << i)) {
                    current_solution[i] = 1;
                    total_weight += items[i].peso;
                    total_value += items[i].valor;
                }
            }

            if (total_weight <= capacity) {
                if (total_value > best_value) {
                    best_value = total_value;
                    best_solutions.clear();
                    best_solutions.push_back(current_solution);
                } else if (total_value == best_value) {
                    best_solutions.push_back(current_solution);
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        return {best_solutions, best_value, duration};
    }

    // Mostrar una solución
    void printSolution(const std::vector<int>& solution) {
        std::cout << "Solución: [";
        for (size_t i = 0; i < solution.size(); ++i) {
            std::cout << solution[i];
            if (i < solution.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;

        int total_weight = 0, total_value = 0;
        std::cout << "Objetos seleccionados: ";
        for (size_t i = 0; i < solution.size(); ++i) {
            if (solution[i] == 1) {
                std::cout << i << "(p=" << items[i].peso << ",v=" << items[i].valor << "), ";
                total_weight += items[i].peso;
                total_value += items[i].valor;
            }
        }
        std::cout << std::endl << "Peso total: " << total_weight << "/" << capacity << std::endl;
        std::cout << "Valor total: " << total_value << std::endl;
    }

    // Mostrar evolución del fitness
    void printEvolution(const std::vector<int>& evolution) {
        for (size_t i = 0; i < evolution.size(); ++i) {
            std::cout << evolution[i] << ",";
        }
        std::cout<< std::endl;
    }
};

int main() {
    
    // PARAMETROS DEL PROBLEMA
    std::vector<Item> items = {
        {7, 70}, {3, 40}, {5, 60}, {8, 80}, {4, 50},
        {6, 55}, {10, 100}, {9, 90}, {2, 30}, {1, 20}
    };
    int capacity = 15;
    // PARAMETROS DEL ALGORITMO
    int nests = 10;
    int maxGenerations = 1000;
    double pa = 0.25;
    double a = 1.0;

    std::cout << std::endl << "=== PROBLEMA ===" << std::endl;
    std::cout << "Capacidad de la mochila: " << capacity;
    std::cout << "\nObjetos disponibles:" << std::endl;
    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << "Item " << i << ": peso=" << items[i].peso 
                 << ", valor=" << items[i].valor << std::endl;
    }

    // Crear instancia del algoritmo
    CuckooSearchKnapsack cuckoo(items, capacity, nests, maxGenerations, pa, a);
    
    
    // Ejecutar Cuckoo Search
    std::cout << "\n=== CUCKOO SEARCH ===" << std::endl;
    auto [best_solution, fitness_evolution, duration_cuckoo] = cuckoo.cuckooSearch();
    // Imprimir Cuckoo Search
    cuckoo.printSolution(best_solution);
    cuckoo.printEvolution(fitness_evolution);
    
    
    /*
    // Ejecutar Cuckoo Search 10 veces
    std::vector<std::vector<int>> all_solutions;
    std::vector<std::vector<int>> all_evolutions;
    std::vector<double> all_times;
    for (int i = 0; i < 10; ++i) {
        auto [best_solution, fitness_evolution, duration_cuckoo] = cuckoo.cuckooSearch();
        all_solutions.push_back(best_solution);
        all_evolutions.push_back(fitness_evolution);
        all_times.push_back(duration_cuckoo.count());
    }
    for (const auto& evo : all_evolutions) {
        cuckoo.printEvolution(evo);
    }
    for (size_t i = 0; i < all_times.size(); ++i) {
        std::cout << all_times[i] << ",";
    }
    */
    
    
    // Ejecutar fuerza bruta para comparación
    /* std::cout << "\n=== FUERZA BRUTA (SOLUCIÓN ÓPTIMA) ===" << std::endl;
    auto [optimal_solutions, optimal_value, duration_bruteforce] = cuckoo.bruteForceSolution();
    // Imprimir fuerza bruta
    std::cout << duration_bruteforce.count() << " segundos" << std::endl;
    std::cout << "Valor óptimo: " << optimal_value << std::endl;
    std::cout << "NúmerSo de soluciones óptimas encontradas: " << optimal_solutions.size() << std::endl;
    cuckoo.printSolution(optimal_solutions[0]); */
    
    
    /*
    // Ejecutar fuerza bruta 10 veces
    std::vector<double> all_times_bruteforce;
    for (int i = 0; i < 10; ++i) {
        auto [optimal_solutions, optimal_value, duration_bruteforce] = cuckoo.bruteForceSolution();
        all_times.push_back(duration_bruteforce.count());
    }
    for (size_t i = 0; i < all_times_bruteforce.size(); ++i) {
        std::cout << all_times_bruteforce[i] << ",";
    }
    */

    return 0;
}