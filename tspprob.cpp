#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <cmath>

using namespace std;

const int num_cities = 10;
const int num_nests = 20;
const int max_iter = 1000;
const double pa = 0.25;

vector<vector<double>> distance_matrix(num_cities, vector<double>(num_cities));
mt19937 rng(time(0));

// Calcular la longitud total del camino
double path_length(const vector<int>& path) {
    double length = 0.0;
    for (int i = 0; i < num_cities - 1; ++i)
        length += distance_matrix[path[i]][path[i+1]];
    length += distance_matrix[path[num_cities - 1]][path[0]]; // vuelta al inicio
    return length;
}

// Inicializar soluciones aleatorias
vector<vector<int>> initialize_nests() {
    vector<vector<int>> nests;
    vector<int> base(num_cities);
    for (int i = 0; i < num_cities; ++i) base[i] = i;

    for (int i = 0; i < num_nests; ++i) {
        shuffle(base.begin(), base.end(), rng);
        nests.push_back(base);
    }
    return nests;
}

// Mutación tipo "Lévy flight" (discreta)
vector<int> levy_flight(const vector<int>& path) {
    vector<int> new_path = path;
    uniform_int_distribution<int> dist(0, num_cities - 1);
    double p = uniform_real_distribution<double>(0, 1)(rng);

    if (p < 0.4) {
        // Swap aleatorio
        int i = dist(rng), j = dist(rng);
        swap(new_path[i], new_path[j]);
    } else if (p < 0.8) {
        // Reversión de segmento
        int i = dist(rng), j = dist(rng);
        if (i > j) swap(i, j);
        reverse(new_path.begin() + i, new_path.begin() + j + 1);
    } else {
        // Reubicación (paso más largo)
        int i = dist(rng), j = dist(rng);
        int city = new_path[i];
        new_path.erase(new_path.begin() + i);
        new_path.insert(new_path.begin() + j, city);
    }

    return new_path;
}

pair<vector<int>, double> cuckoo_search_tsp(int num_cities, int num_nests, int max_iter, double pa) {
    auto nests = initialize_nests();
    vector<int> best_path = nests[0];
    double best_length = path_length(best_path);

    for (int iter = 0; iter < max_iter; ++iter) {
        for (int i = 0; i < num_nests; ++i) {
            // Generar nuevo huevo por Lévy Flight
            auto new_path = levy_flight(nests[i]);
            double new_length = path_length(new_path);

            // Reemplazo si mejora
            if (new_length < path_length(nests[i])) {
                nests[i] = new_path;

                // Actualizar mejor solución global
                if (new_length < best_length) {
                    best_length = new_length;
                    best_path = new_path;
                }
            }
        }

        // Abandono aleatorio de nidos (exploración)
        for (int i = 0; i < num_nests; ++i) {
            if (uniform_real_distribution<double>(0, 1)(rng) < pa) {
                vector<int> base(num_cities);
                for (int j = 0; j < num_cities; ++j) base[j] = j;
                shuffle(base.begin(), base.end(), rng);
                nests[i] = base;
            }
        }

        // (Opcional) Mostrar progreso:
        // if (iter % 100 == 0) cout << "Iter " << iter << " mejor distancia: " << best_length << endl;
    }

    return {best_path, best_length};
}

void mostrar_ruta(const vector<int>& ruta) {
    double total = 0;
    cout << "Ruta:" << endl;
    for (size_t i = 0; i < ruta.size(); ++i) {
        int from = ruta[i];
        int to = ruta[(i + 1) % ruta.size()];
        double dist = distance_matrix[from][to];
        cout << from << " -> " << to << " : " << dist << endl;
        total += dist;
    }
    cout << "Distancia total: " << total << endl;
}


int main() {
    // Inicializamos la matriz de distancias aleatorias
    uniform_real_distribution<double> dist(1.0, 100.0);
    for (int i = 0; i < num_cities; ++i)
        for (int j = i + 1; j < num_cities; ++j)
            distance_matrix[i][j] = distance_matrix[j][i] = dist(rng);

    auto [best_path, best_length] = cuckoo_search_tsp(num_cities, num_nests, max_iter, pa);

    cout << "Mejor recorrido encontrado: ";
    for (int city : best_path) cout << city << " ";
    cout << best_path[0] << endl;
    cout << "Longitud total: " << best_length << endl;

    mostrar_ruta(best_path);


    return 0;
}
