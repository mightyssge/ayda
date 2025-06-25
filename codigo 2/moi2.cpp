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

// Clase que implementa el algoritmo Cuckoo Search para el problema de la mochila
class CuckooSearchKnapsack {
private:
    std::vector<Item> items;         // Lista de ítems
    int capacity;                    // Capacidad de la mochila
    int n_nests;                     // Número de nidos (soluciones)
    int max_iter;                    // Máximo número de iteraciones
    double pa;                       // Proporción de nidos a reemplazar (abandono)
    double alfa;                     // Factor de escala para Lévy flight

    // Generadores aleatorios
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;
    std::uniform_int_distribution<> binary_dis;
    std::normal_distribution<> normal_dis;

public:
    // Constructor de la clase
    CuckooSearchKnapsack(const std::vector<Item>& items, int cap, int nests, int iterations, double prob_abandon, double a)
        : items(items), capacity(cap), n_nests(nests), max_iter(iterations), pa(prob_abandon), alfa(a),
          gen(std::random_device{}()), dis(0.0, 1.0), binary_dis(0, 1), normal_dis(0.0, 1.0) {}

    // Calcula el fitness de una solución (valor total si respeta la capacidad)
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

    // Genera una solución aleatoria binaria(nido)
    std::vector<int> generateRandomSolution() {
        std::vector<int> nest(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            nest[i] = binary_dis(gen);
        }
        return nest;
    }

    // Realiza un Levy Flight para generar una nueva solución basada en la actual
    // Adaptación: La solución es binaria {0,1} y se usa una función sigmoide para convertir el valor continuo a binario
    // Nota: El parámetro alfa controla la escala del Levy Flight
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

    // Implementa el algoritmo Cuckoo Search
    // Retorna la mejor solución encontrada, la evolución del fitness y el tiempo de ejecución
    std::tuple<std::vector<int>, std::vector<int>, std::chrono::duration<double>> cuckooSearch() {
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<std::vector<int>> nests(n_nests);
        std::vector<int> fitness_values(n_nests);

        // Inicializar nidos con soluciones aleatorias
        for (int i = 0; i < n_nests; ++i) {
            nests[i] = generateRandomSolution();
            fitness_values[i] = fitness(nests[i]);
        }

        // Encontrar el mejor nido inicial
        int best_idx = std::max_element(fitness_values.begin(), fitness_values.end()) - fitness_values.begin();
        std::vector<int> best_nest = nests[best_idx];
        int best_fitness = fitness_values[best_idx];
        std::vector<int> fitness_evolution;
        fitness_evolution.push_back(best_fitness);


        int iteration = 0;
        while (iteration < max_iter) { 

            std::uniform_int_distribution<> nest_dis(0, n_nests - 1);
            // Seleccionar un nido aleatorio y aplicar Levy Flight
            // Luego comparar con otro nido aleatorio para decidir si se acepta o no
            int i = nest_dis(gen);
            std::vector<int> new_nest = levyFlight(nests[i]);
            int new_fitness = fitness(new_nest);
            int j;
            do {
                j = nest_dis(gen);
            } while (j == i);

            // Decidir si se acepta el nuevo nido basado en su fitness
            // Si el nuevo nido tiene mejor fitness que el nido j, se acepta
            std::string decision = "Rechazada";
            int fitness_j_before = fitness_values[j];
            if (new_fitness > fitness_values[j]) {
                nests[j] = new_nest;
                fitness_values[j] = new_fitness;
                decision = "Aceptada";
            }

            // Actualizar el mejor nido si es necesario
            // Se busca el nido con mejor fitness entre todos los nidos
            int new_best_idx = std::max_element(fitness_values.begin(), fitness_values.end()) - fitness_values.begin();
            if (fitness_values[new_best_idx] > best_fitness ) {
                best_fitness = fitness_values[new_best_idx];
                best_nest = nests[new_best_idx];

                std::cout << "\nNuevo mejor fitness: " << best_fitness << " encontrado en la iteracion " << iteration << std::endl;
                for (int n = 0; n < n_nests; ++n) {
                    std::cout << "Nido " << n << ": [";
                    for (size_t b = 0; b < nests[n].size(); ++b) {
                        std::cout << nests[n][b];
                        if (b < nests[n].size() - 1) std::cout << ", ";
                    }
                    std::cout << "] -> Fitness: " << fitness_values[n] << std::endl;
                }
                std::cout << "Se uso el nido " << i << " para crear una nueva solucion (";
                for (size_t b = 0; b < new_nest.size(); ++b) {
                    std::cout << new_nest[b];
                    if (b < new_nest.size() - 1) std::cout << ", ";
                }
                std::cout << "). Comparado con nido " << j
                          << ", decision: " << decision
                          << " (fitness nuevo: " << new_fitness
                          << ", fitness j previo: " << fitness_j_before << ")" << std::endl;
            }

            // Reemplazar nidos con peor fitness
            // Se reemplazan los nidos con peor fitness con nuevas soluciones aleatorias
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

    void printSolution(const std::vector<int>& solution) {
        std::cout << "Solucion: [";
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

    void printEvolution(const std::vector<int>& evolution) {
        for (size_t i = 0; i < evolution.size(); ++i) {
            std::cout << evolution[i] << ",";
        }
        std::cout << std::endl;
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

    std::cout << std::endl << "=== PROBLEMA ===" << std::endl;
    std::cout << "Capacidad de la mochila: " << capacity;
    std::cout << "\nObjetos disponibles:" << std::endl;
    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << "Item " << i << ": peso=" << items[i].peso 
                  << ", valor=" << items[i].valor << std::endl;
    }

    CuckooSearchKnapsack cuckoo(items, capacity, nests, maxGenerations, pa, a);

    std::cout << "\n=== CUCKOO SEARCH ===" << std::endl;
    auto [best_solution, fitness_evolution, duration_cuckoo] = cuckoo.cuckooSearch();
    std::cout << "=======================" << std::endl;
    std::cout << "Tiempo de ejecucion: " << std::fixed << std::setprecision(4) << duration_cuckoo.count() << " segundos" << std::endl;
    cuckoo.printSolution(best_solution);
    std::cout << "=======================" << std::endl;
    cuckoo.printEvolution(fitness_evolution);

    return 0;
}
