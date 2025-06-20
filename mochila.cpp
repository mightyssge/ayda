#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <random> 

using namespace std;
using namespace chrono;

random_device rd;
mt19937 g(rd());


// Estructura del objeto
struct Item {
    int id;
    int value;
    int weight;
};

// Parámetros del algoritmo
const int n_nests = 15;
const double pa = 0.25;
const int max_generations = 50;

// Variables globales
int n_items;
int capacity;
vector<Item> items;

// Funciones auxiliares
int fitness(const vector<int>& x) {
    int total_weight = 0, total_value = 0;
    for (int i = 0; i < n_items; ++i) {
        if (x[i]) {
            total_weight += items[i].weight;
            total_value += items[i].value;
        }
    }
    return (total_weight <= capacity) ? total_value : 0;
}

vector<int> random_solution() {
    vector<int> x(n_items, 0);
    int total_weight = 0;
    vector<int> indices(n_items);
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), g);

    for (int i : indices) {
        if (total_weight + items[i].weight <= capacity) {
            x[i] = 1;
            total_weight += items[i].weight;
        }
    }
    return x;
}

vector<int> levy_mutation(const vector<int>& x) {
    vector<int> x_new = x;
    for (int i = 0; i < n_items; ++i) {
        if ((double)rand() / RAND_MAX < 1.0 / n_items)
            x_new[i] = 1 - x[i];
    }

    int total_weight = 0;
    for (int i = 0; i < n_items; ++i)
        if (x_new[i]) total_weight += items[i].weight;

    if (total_weight > capacity) {
        vector<int> indices(n_items);
        iota(indices.begin(), indices.end(), 0);
        shuffle(indices.begin(), indices.end(), g);

        for (int i : indices) {
            if (x_new[i]) {
                x_new[i] = 0;
                total_weight -= items[i].weight;
                if (total_weight <= capacity) break;
            }
        }
    }

    return x_new;
}

void initialize_items(int size) {
    items.clear();
    for (int i = 0; i < size; ++i) {
        int val = rand() % 100 + 1;
        int wgt = rand() % 50 + 1;
        items.push_back({i + 1, val, wgt});
    }
    capacity = size * 10 / 3; // capacidad proporcional
    n_items = size;
    cout << "\nItems generados (ID, valor, peso):" << endl;
    for (int i = 0; i < n_items; ++i) {
        cout << "Item " << items[i].id << ": Valor = " << items[i].value << ", Peso = " << items[i].weight << endl;
    }
}

int main() {
    srand(time(0));

    cout << "\n--- CUCKOO SEARCH PARA PROBLEMA DE LA MOCHILA ---\n";
    cout << "Elige el tamaño del problema:\n1. 10 objetos\n2. 100 objetos\n3. 1000 objetos\nOpcion: ";

    int opcion;
    cin >> opcion;
    if (opcion == 1) initialize_items(10);
    else if (opcion == 2) initialize_items(100);
    else initialize_items(1000);

    cout << "\nCapacidad de la mochila: " << capacity << endl;

    vector<vector<int>> nests(n_nests);
    vector<int> scores(n_nests);

    for (int i = 0; i < n_nests; ++i) {
        nests[i] = random_solution();
        scores[i] = fitness(nests[i]);
    }

    auto start = high_resolution_clock::now();

    for (int gen = 0; gen < max_generations; ++gen) {
        cout << "\nGeneracion " << gen + 1 << "\n-----------------------------" << endl;
        for (int i = 0; i < n_nests; ++i) {
            vector<int> cuckoo = levy_mutation(nests[i]);
            int cuckoo_score = fitness(cuckoo);
            int j = rand() % n_nests;
            if (cuckoo_score > scores[j]) {
                cout << "Nido " << j << " reemplazado. Valor anterior: " << scores[j] << ", nuevo valor: " << cuckoo_score << endl;
                nests[j] = cuckoo;
                scores[j] = cuckoo_score;
            }
        }

        int n_replace = (int)(pa * n_nests);
        vector<pair<int, int>> indexed_scores;
        for (int i = 0; i < n_nests; ++i)
            indexed_scores.push_back({scores[i], i});

        sort(indexed_scores.begin(), indexed_scores.end());
        for (int k = 0; k < n_replace; ++k) {
            int idx = indexed_scores[k].second;
            nests[idx] = random_solution();
            scores[idx] = fitness(nests[idx]);
            cout << "Nido " << idx << " reemplazado aleatoriamente por bajo rendimiento." << endl;
        }

        int best = max_element(scores.begin(), scores.end()) - scores.begin();
        int total_weight = 0;
        cout << "Contenido de mejor nido: [ ";
        for (int i = 0; i < n_items; ++i) {
            if (nests[best][i]) {
                cout << items[i].id << " ";
                total_weight += items[i].weight;
            }
        }
        cout << "]\nMejor valor: " << scores[best] << ", Peso: " << total_weight << endl;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    int best = max_element(scores.begin(), scores.end()) - scores.begin();
    cout << "\n===============================" << endl;
    cout << "\n✅ Mejor solucion encontrada:" << endl;
    cout << "Valor total: " << scores[best] << endl;
    cout << "Peso total: ";
    int final_weight = 0;
    for (int i = 0; i < n_items; ++i)
        if (nests[best][i]) final_weight += items[i].weight;
    cout << final_weight << endl;
    cout << "Tiempo de ejecucion: " << duration.count() << " ms" << endl;
    cout << "\n===============================\n";

    return 0;
}
