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

// Problem: 07.05

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>

namespace hybrid_sort {

template <typename T>
void insertion_sort(std::vector<T>& data, std::size_t left, std::size_t right) {
    for (std::size_t i = left + 1; i < right; ++i) {
        for (std::size_t j = i; j > left; --j) {
            if (data[j - 1] > data[j]) {
                std::swap(data[j], data[j - 1]);
            }
        }
    }
}

template <typename T>
std::size_t partition_hoare(std::vector<T>& data, std::size_t left, std::size_t right) {
    const std::size_t mid = std::midpoint(left, right - 1);

    if (data[mid] < data[left]) {
        std::swap(data[mid], data[left]);
    }
    if (data[right - 1] < data[left]) {
        std::swap(data[right - 1], data[left]);
    }
    if (data[right - 1] < data[mid]) {
        std::swap(data[right - 1], data[mid]);
    }

    const T pivot = data[mid];

    std::size_t i = left;
    std::size_t j = right - 1;

    while (true) {
        while (data[i] < pivot) {
            ++i;
        }
        while (data[j] > pivot) {
            --j;
        }

        if (i >= j) {
            return j + 1;
        }

        std::swap(data[i], data[j]);
        ++i;
        --j;
    }
}

template <typename T>
void quick_sort_impl(std::vector<T>& data, std::size_t left, std::size_t right,
                     std::size_t insertion_threshold) {
    if (right - left > insertion_threshold) {
        const std::size_t split = partition_hoare(data, left, right);
        quick_sort_impl(data, left, split, insertion_threshold);
        quick_sort_impl(data, split, right, insertion_threshold);
    } else {
        insertion_sort(data, left, right);
    }
}

template <typename T>
void sort(std::vector<T>& data, std::size_t insertion_threshold) {
    if (data.empty()) {
        return;
    }
    quick_sort_impl(data, 0, data.size(), insertion_threshold);
}

}  // namespace hybrid_sort

namespace benchmark_data {

std::vector<double> make_reverse_sorted_vector() {
    std::vector<double> data(10000);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<double>(data.size() - i);
    }
    return data;
}

}  // namespace benchmark_data

// threshold is the segment size below which the algorithm stops recursing
// with quick sort and finishes the segment with insertion sort instead.
static void benchmark_hybrid_sort(benchmark::State& state) {
    const std::size_t threshold = static_cast<std::size_t>(state.range(0));
    const std::vector<double> source = benchmark_data::make_reverse_sorted_vector();

    {
        std::vector<double> check = source;
        hybrid_sort::sort(check, threshold);
        if (!std::is_sorted(check.begin(), check.end())) {
            state.SkipWithError("sort() failed to produce ordered data");
            return;
        }
    }

    for (auto _ : state) {
        state.PauseTiming();
        std::vector<double> data = source;
        state.ResumeTiming();

        hybrid_sort::sort(data, threshold);

        benchmark::DoNotOptimize(data.data());
        benchmark::ClobberMemory();
    }

    state.SetLabel("threshold=" + std::to_string(threshold));
}

BENCHMARK(benchmark_hybrid_sort)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64);

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
