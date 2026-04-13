#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out" -lbenchmark -pthread
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 10.06
// Description: Fibonacci numbers via matrix exponentiation with Boost.Ublas matrices.

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>
#include <boost/numeric/ublas/matrix.hpp>

using matrix_t = boost::numeric::ublas::matrix<unsigned long long>;

matrix_t multiply(const matrix_t& lhs, const matrix_t& rhs) {
    matrix_t result(2, 2);

    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
            result(i, j) = 0;
            for (std::size_t k = 0; k < 2; ++k) {
                result(i, j) += lhs(i, k) * rhs(k, j);
            }
        }
    }

    return result;
}

matrix_t identity_matrix() {
    matrix_t identity(2, 2);
    identity(0, 0) = 1;
    identity(0, 1) = 0;
    identity(1, 0) = 0;
    identity(1, 1) = 1;
    return identity;
}

matrix_t fibonacci_base_matrix() {
    matrix_t base(2, 2);
    base(0, 0) = 1;
    base(0, 1) = 1;
    base(1, 0) = 1;
    base(1, 1) = 0;
    return base;
}

matrix_t power(matrix_t base, unsigned long long exponent) {
    matrix_t result = identity_matrix();

    while (exponent > 0) {
        if ((exponent & 1ULL) != 0ULL) {
            result = multiply(result, base);
        }
        base = multiply(base, base);
        exponent >>= 1ULL;
    }

    return result;
}

unsigned long long fibonacci_matrix(unsigned long long n) {
    if (n == 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }

    const matrix_t matrix = power(fibonacci_base_matrix(), n - 1);
    return matrix(0, 0);
}

unsigned long long fibonacci_linear(unsigned long long n) {
    if (n == 0) {
        return 0;
    }

    unsigned long long current = 0;
    unsigned long long next = 1;

    for (unsigned long long i = 0; i < n; ++i) {
        const unsigned long long updated = current + next;
        current = next;
        next = updated;
    }

    return current;
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

void test_known_values(Logger& log) {
    const std::vector<unsigned long long> expected{
        0ULL, 1ULL, 1ULL, 2ULL, 3ULL, 5ULL, 8ULL, 13ULL, 21ULL, 34ULL, 55ULL,
    };

    for (std::size_t i = 0; i < expected.size(); ++i) {
        require(fibonacci_matrix(i) == expected[i], "known Fibonacci value mismatch");
    }

    log.require(true, "known Fibonacci values are correct");
}

void test_matches_linear_algorithm(Logger& log) {
    for (unsigned long long n = 0; n <= 50; ++n) {
        require(fibonacci_matrix(n) == fibonacci_linear(n), "matrix and linear algorithms must match");
    }

    log.require(true, "matrix exponentiation matches linear algorithm");
}

void test_power_identity(Logger& log) {
    const matrix_t result = power(fibonacci_base_matrix(), 0);
    require(result(0, 0) == 1 && result(0, 1) == 0 && result(1, 0) == 0 && result(1, 1) == 1,
            "zero power should give identity");
    log.require(true, "matrix power handles zero exponent");
}

void test_large_safe_value(Logger& log) {
    require(fibonacci_matrix(93) == 12200160415121876738ULL, "F(93) should fit in unsigned long long");
    log.require(true, "largest commonly safe Fibonacci value is correct");
}

void run_all() {
    Logger log;

    try {
        test_known_values(log);
        test_matches_linear_algorithm(log);
        test_power_identity(log);
        test_large_safe_value(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

static void benchmark_fibonacci_matrix(benchmark::State& state) {
    const unsigned long long n = static_cast<unsigned long long>(state.range(0));

    for (auto _ : state) {
        benchmark::DoNotOptimize(fibonacci_matrix(n));
    }

    state.SetComplexityN(static_cast<benchmark::IterationCount>(n));
}

static void benchmark_fibonacci_linear(benchmark::State& state) {
    const unsigned long long n = static_cast<unsigned long long>(state.range(0));

    for (auto _ : state) {
        benchmark::DoNotOptimize(fibonacci_linear(n));
    }

    state.SetComplexityN(static_cast<benchmark::IterationCount>(n));
}

BENCHMARK(benchmark_fibonacci_matrix)->DenseRange(5, 90, 5)->Complexity(benchmark::oLogN);
BENCHMARK(benchmark_fibonacci_linear)->DenseRange(5, 90, 5)->Complexity(benchmark::oN);

int main(int argc, char** argv) {
    tests::run_all();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
