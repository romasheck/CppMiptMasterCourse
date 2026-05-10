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

// Problem: 11.02
// Description: Google.Benchmark comparison of six callable forms.

#include <benchmark/benchmark.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

#if defined(__GNUC__) || defined(__clang__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

NOINLINE int free_function(int value) {
    return value + 1;
}

class Base {
public:
    virtual ~Base() = default;

    NOINLINE virtual int call(int value) const {
        return value + 1;
    }
};

class Derived final : public Base {
public:
    NOINLINE int member(int value) const {
        return value + 1;
    }

    NOINLINE int call(int value) const override {
        return value + 1;
    }
};

struct FunctionObject {
    NOINLINE int operator()(int value) const {
        return value + 1;
    }
};

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

void test_callables_return_same_value(Logger& log) {
    Derived object;
    const Base& base = object;
    FunctionObject function_object;
    auto lambda_auto = [](int value) NOINLINE {
        return value + 1;
    };
    std::function<int(int)> lambda_function = [](int value) NOINLINE {
        return value + 1;
    };

    require(free_function(41) == 42, "free function");
    require(object.member(41) == 42, "member function");
    require(base.call(41) == 42, "virtual member function");
    require(function_object(41) == 42, "function object");
    require(lambda_auto(41) == 42, "auto lambda");
    require(lambda_function(41) == 42, "std::function lambda");

    log.require(true, "all benchmarked callables have identical behavior");
}

void run_all() {
    Logger log;

    try {
        test_callables_return_same_value(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

static void benchmark_free_function(benchmark::State& state) {
    int value = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(value = free_function(value));
    }
}

static void benchmark_member_function(benchmark::State& state) {
    Derived object;
    int value = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(value = object.member(value));
    }
}

static void benchmark_virtual_function(benchmark::State& state) {
    Derived object;
    const Base& base = object;
    int value = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(value = base.call(value));
    }
}

static void benchmark_function_object(benchmark::State& state) {
    FunctionObject object;
    int value = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(value = object(value));
    }
}

static void benchmark_auto_lambda(benchmark::State& state) {
    auto lambda = [](int value) NOINLINE {
        return value + 1;
    };
    int value = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(value = lambda(value));
    }
}

static void benchmark_std_function_lambda(benchmark::State& state) {
    std::function<int(int)> lambda = [](int value) NOINLINE {
        return value + 1;
    };
    int value = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(value = lambda(value));
    }
}

BENCHMARK(benchmark_free_function);
BENCHMARK(benchmark_member_function);
BENCHMARK(benchmark_virtual_function);
BENCHMARK(benchmark_function_object);
BENCHMARK(benchmark_auto_lambda);
BENCHMARK(benchmark_std_function_lambda);

int main(int argc, char** argv) {
    tests::run_all();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
