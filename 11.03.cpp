#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 11.03
// Description: Brute-force traveling salesman for a complete graph with 10 vertices.


// No the task is different
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Matrix = std::vector<std::vector<int>>;

struct TourResult {
    std::vector<int> route;
    int cost = std::numeric_limits<int>::max();
};

Matrix make_random_complete_graph(int vertices, int min_weight, int max_weight, unsigned seed) {
    Matrix matrix(vertices, std::vector<int>(vertices, 0));
    std::mt19937 engine(seed);
    std::uniform_int_distribution<int> distribution(min_weight, max_weight);

    for (int i = 0; i < vertices; ++i) {
        for (int j = i + 1; j < vertices; ++j) {
            const int weight = distribution(engine);
            matrix[i][j] = weight;
            matrix[j][i] = weight;
        }
    }

    return matrix;
}

int route_cost(const Matrix& matrix, const std::vector<int>& order) {
    if (order.empty()) {
        return 0;
    }

    int cost = 0;
    for (std::size_t i = 1; i < order.size(); ++i) {
        cost += matrix[order[i - 1]][order[i]];
    }
    cost += matrix[order.back()][order.front()];
    return cost;
}

TourResult solve_tsp(const Matrix& matrix) {
    const int vertices = static_cast<int>(matrix.size());
    std::vector<int> permutation(vertices - 1);
    std::iota(permutation.begin(), permutation.end(), 1);

    TourResult best;

    do {
        std::vector<int> route;
        route.reserve(vertices + 1);
        route.push_back(0);
        route.insert(route.end(), permutation.begin(), permutation.end());
        route.push_back(0);

        int cost = 0;
        for (std::size_t i = 1; i < route.size(); ++i) {
            cost += matrix[route[i - 1]][route[i]];
        }

        if (cost < best.cost) {
            best.cost = cost;
            best.route = route;
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));

    return best;
}

void print_matrix(const Matrix& matrix) {
    std::cout << "Incidence matrix:\n";
    for (const auto& row : matrix) {
        for (int value : row) {
            std::cout << std::setw(3) << value;
        }
        std::cout << '\n';
    }
}

void print_route(const std::vector<int>& route) {
    std::cout << "Optimal route: ";
    for (std::size_t i = 0; i < route.size(); ++i) {
        if (i != 0) {
            std::cout << " -> ";
        }
        std::cout << route[i];
    }
    std::cout << '\n';
}

namespace tests {

struct Logger {
    int passed = 0;
    int failed = 0;

    void require(bool condition, const char* message) {
        if (condition) {
            ++passed;
            std::cout << "[OK]   " << message << '\n';
        } else {
            ++failed;
            std::cout << "[FAIL] " << message << '\n';
        }
    }
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_symmetric_matrix(Logger& log) {
    const Matrix matrix = make_random_complete_graph(10, 1, 10, 123456u);

    for (int i = 0; i < 10; ++i) {
        require(matrix[i][i] == 0, "diagonal should be zero");
        for (int j = 0; j < 10; ++j) {
            require(matrix[i][j] == matrix[j][i], "matrix should be symmetric");
        }
    }

    log.require(true, "random graph is symmetric");
}

void test_known_small_graph(Logger& log) {
    const Matrix matrix{
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0},
    };

    const TourResult result = solve_tsp(matrix);
    require(result.cost == 80, "expected optimal cost 80");
    require(result.route.front() == 0 && result.route.back() == 0, "route should return to start");
    log.require(true, "solver finds optimal route on a known graph");
}

void test_route_cost(Logger& log) {
    const Matrix matrix{
        {0, 2, 9},
        {2, 0, 3},
        {9, 3, 0},
    };
    const std::vector<int> route{0, 1, 2};

    require(route_cost(matrix, route) == 14, "route cost should include return to start");
    log.require(true, "route cost counts closing edge");
}

void run_all() {
    Logger log;

    try {
        test_symmetric_matrix(log);
        test_known_small_graph(log);
        test_route_cost(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();

    constexpr int vertices = 10;
    const Matrix matrix = make_random_complete_graph(vertices, 1, 10, 20250414u);
    const TourResult result = solve_tsp(matrix);

    std::cout << '\n';
    print_matrix(matrix);
    print_route(result.route);
    std::cout << "Total cost: " << result.cost << '\n';

    return 0;
}
