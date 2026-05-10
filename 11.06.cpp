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

// Problem: 11.06
// Description: Traveling salesman for a complete graph stored as Boost.Graph adjacency_matrix.

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/properties.hpp>

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

using Graph = boost::adjacency_matrix<boost::undirectedS,
                                      boost::no_property,
                                      boost::property<boost::edge_weight_t, int>>;
using WeightMap = boost::property_map<Graph, boost::edge_weight_t>::type;

struct TourResult {
    std::vector<int> order;
    int cost = std::numeric_limits<int>::max();
};

Graph make_graph(int vertices, std::default_random_engine& engine) {
    Graph graph(vertices);
    WeightMap weights = boost::get(boost::edge_weight, graph);
    std::uniform_int_distribution<int> distribution(1, 10);

    for (int i = 0; i < vertices; ++i) {
        for (int j = i + 1; j < vertices; ++j) {
            const auto [edge, inserted] = boost::add_edge(i, j, graph);
            if (!inserted) {
                throw std::runtime_error("edge insertion failed");
            }
            weights[edge] = distribution(engine);
        }
    }

    return graph;
}

Graph make_graph_from_matrix(const std::vector<std::vector<int>>& matrix) {
    Graph graph(static_cast<int>(matrix.size()));
    WeightMap weights = boost::get(boost::edge_weight, graph);

    for (std::size_t i = 0; i < matrix.size(); ++i) {
        for (std::size_t j = i + 1; j < matrix.size(); ++j) {
            const auto [edge, inserted] = boost::add_edge(i, j, graph);
            if (!inserted) {
                throw std::runtime_error("edge insertion failed");
            }
            weights[edge] = matrix[i][j];
        }
    }

    return graph;
}

int edge_weight(const Graph& graph, int from, int to) {
    const auto [edge, exists] = boost::edge(from, to, graph);
    if (!exists) {
        throw std::runtime_error("missing edge");
    }

    const auto weights = boost::get(boost::edge_weight, graph);
    return weights[edge];
}

int calculate_cycle_cost(const Graph& graph, const std::vector<int>& order) {
    int cost = 0;

    for (std::size_t i = 1; i < order.size(); ++i) {
        cost += edge_weight(graph, order[i - 1], order[i]);
    }

    cost += edge_weight(graph, order.back(), order.front());
    return cost;
}

TourResult solve(const Graph& graph) {
    const int vertices = static_cast<int>(boost::num_vertices(graph));
    std::vector<int> order(vertices);
    std::iota(order.begin(), order.end(), 0);

    TourResult best;

    do {
        const int current_cost = calculate_cycle_cost(graph, order);
        if (current_cost < best.cost) {
            best.cost = current_cost;
            best.order = order;
        }
    } while (std::next_permutation(std::next(order.begin()), order.end()));

    return best;
}

void print_matrix(const Graph& graph) {
    const int vertices = static_cast<int>(boost::num_vertices(graph));

    std::cout << "Incidence matrix:\n";
    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            std::cout << std::setw(3) << (i == j ? 0 : edge_weight(graph, i, j));
        }
        std::cout << '\n';
    }
}

void print_route(const std::vector<int>& order) {
    std::cout << "Optimal order: ";
    for (std::size_t i = 0; i < order.size(); ++i) {
        if (i != 0) {
            std::cout << " -> ";
        }
        std::cout << order[i];
    }
    std::cout << " -> " << order.front() << '\n';
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

void test_known_graph(Logger& log) {
    const Graph graph = make_graph_from_matrix({
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0},
    });

    const TourResult result = solve(graph);
    require(result.cost == 80, "known graph optimum should be 80");
    require(result.order.size() == 4, "order should contain each vertex once");
    log.require(true, "solver finds optimal Hamiltonian cycle");
}

void test_generated_graph(Logger& log) {
    std::default_random_engine engine(123456u);
    const Graph graph = make_graph(10, engine);

    for (int i = 0; i < 10; ++i) {
        for (int j = i + 1; j < 10; ++j) {
            const int weight = edge_weight(graph, i, j);
            require(1 <= weight && weight <= 10, "edge weights should be in requested range");
            require(edge_weight(graph, i, j) == edge_weight(graph, j, i), "undirected graph weights");
        }
    }

    log.require(true, "random complete graph is generated");
}

void test_cycle_cost(Logger& log) {
    const Graph graph = make_graph_from_matrix({
        {0, 2, 9},
        {2, 0, 3},
        {9, 3, 0},
    });

    require(calculate_cycle_cost(graph, {0, 1, 2}) == 14, "cycle cost should include return edge");
    log.require(true, "cycle cost includes return to start");
}

void run_all() {
    Logger log;

    try {
        test_known_graph(log);
        test_generated_graph(log);
        test_cycle_cost(log);
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

    std::random_device device;
    std::default_random_engine engine(device());
    const Graph graph = make_graph(10, engine);
    const TourResult result = solve(graph);

    std::cout << '\n';
    print_matrix(graph);
    print_route(result.order);
    std::cout << "Total cost: " << result.cost << '\n';

    return 0;
}
