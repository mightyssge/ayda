#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <ctime>
#include <algorithm>

using namespace std;

struct Item {
    int value;
    int weight;
};

struct Solution {
    vector<int> x;
    int fitness;
};

// Parámetros
const int num_items = 10;
const int capacity = 35;
const int num_nests = 15;
const int max_iter = 100;
const double pa = 0.25;

// Items ejemplo
vector<Item> items = {
    {12, 7}, {10, 5}, {8, 6}, {11, 7}, {14, 3},
    {7, 2}, {9, 4}, {13, 6}, {15, 8}, {6, 3}
};

// Random
mt19937 rng(time(0));
uniform_real_distribution<double> uniform(0.0, 1.0);
uniform_int_distribution<int> binary(0, 1);

// Fitness function
int evaluate(const vector<int>& x) {
    int total_value = 0, total_weight = 0;
    for (int i = 0; i < num_items; ++i) {
        if (x[i]) {
            total_value += items[i].value;
            total_weight += items[i].weight;
        }
    }
    if (total_weight > capacity) return 0;
    return total_value;
}

// Lévy Flight para soluciones binarias
vector<int> levy_binary(const vector<int>& x) {
    vector<int> new_x = x;
    for (int i = 0; i < num_items; ++i) {
        if (uniform(rng) < 0.5) {
            new_x[i] = 1 - x[i];  // flip bit con probabilidad
        }
    }
    return new_x;
}

// Inicialización aleatoria de nidos
vector<Solution> initialize_nests() {
    vector<Solution> nests;
    for (int i = 0; i < num_nests; ++i) {
        vector<int> x(num_items);
        for (int j = 0; j < num_items; ++j)
            x[j] = binary(rng);
        nests.push_back({x, evaluate(x)});
    }
    return nests;
}

int main() {
    vector<Solution> nests = initialize_nests();
    Solution best = *max_element(nests.begin(), nests.end(),
                                 [](const Solution& a, const Solution& b) {
                                     return a.fitness < b.fitness;
                                 });

    for (int iter = 0; iter < max_iter; ++iter) {
        for (int i = 0; i < num_nests; ++i) {
            vector<int> new_x = levy_binary(nests[i].x);
            int new_fitness = evaluate(new_x);

            if (new_fitness > nests[i].fitness) {
                nests[i] = {new_x, new_fitness};
                if (new_fitness > best.fitness) {
                    best = nests[i];
                }
            }
        }

        // Descubrimiento de nidos con probabilidad pa
        for (int i = 0; i < num_nests; ++i) {
            if (uniform(rng) < pa) {
                vector<int> x(num_items);
                for (int j = 0; j < num_items; ++j)
                    x[j] = binary(rng);
                nests[i] = {x, evaluate(x)};
            }
        }
    }

    // Mostrar solución
    cout << "Mejor valor: " << best.fitness << endl;
    cout << "Objetos seleccionados: ";
    for (int i = 0; i < num_items; ++i) {
        if (best.x[i]) cout << i << " ";
    }
    cout << endl ;

    return 0;
}
