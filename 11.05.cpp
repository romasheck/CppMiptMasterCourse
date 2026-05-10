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

// Problem: 11.05
// Description: Ranges algorithms, views, transform_if, errors, and a Fibonacci view.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

template <std::ranges::input_range Range, typename Predicate, typename Operation, typename Output>
Output transform_if(Range&& range, Output output, Predicate predicate, Operation operation) {
    std::vector<std::ranges::range_value_t<Range>> filtered;
    std::ranges::copy_if(range, std::back_inserter(filtered), predicate);
    return std::ranges::transform(filtered, output, operation).out;
}

double mae(const std::vector<double>& expected, const std::vector<double>& actual) {
    if (expected.size() != actual.size() || expected.empty()) {
        throw std::invalid_argument("vectors must have the same non-zero size");
    }

    const double sum = std::transform_reduce(expected.begin(), expected.end(), actual.begin(), 0.0,
                                             std::plus<>{}, [](double lhs, double rhs) {
                                                 return std::abs(lhs - rhs);
                                             });
    return sum / static_cast<double>(expected.size());
}

double mse(const std::vector<double>& expected, const std::vector<double>& actual) {
    if (expected.size() != actual.size() || expected.empty()) {
        throw std::invalid_argument("vectors must have the same non-zero size");
    }

    const double sum = std::transform_reduce(expected.begin(), expected.end(), actual.begin(), 0.0,
                                             std::plus<>{}, [](double lhs, double rhs) {
                                                 const double diff = lhs - rhs;
                                                 return diff * diff;
                                             });
    return sum / static_cast<double>(expected.size());
}

class Fibonacci : public std::ranges::view_interface<Fibonacci> {
public:
    explicit Fibonacci(std::size_t count) : count_(count) {
    }

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;
        explicit Iterator(std::size_t count) : count_(count) {
        }

        int operator*() const {
            return current_;
        }

        Iterator& operator++() {
            const int updated = current_ + next_;
            current_ = next_;
            next_ = updated;
            ++index_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator copy = *this;
            ++(*this);
            return copy;
        }

        friend bool operator==(const Iterator& iterator, std::default_sentinel_t) {
            return iterator.index_ >= iterator.count_;
        }

    private:
        std::size_t count_ = 0;
        std::size_t index_ = 0;
        int current_ = 0;
        int next_ = 1;
    };

    Iterator begin() const {
        return Iterator(count_);
    }

    std::default_sentinel_t end() const {
        return {};
    }

private:
    std::size_t count_ = 0;
};

template <std::ranges::input_range Range>
auto collect(Range&& range) {
    std::vector<std::ranges::range_value_t<Range>> result;
    for (auto&& value : range) {
        result.push_back(value);
    }
    return result;
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

void test_ranges_algorithms(Logger& log) {
    std::vector<int> values{1, 2, 2, 3, 4};
    std::ranges::replace(values, 2, 9);
    require(values == std::vector<int>({1, 9, 9, 3, 4}), "ranges::replace");

    std::ranges::fill(values, 7);
    require(values == std::vector<int>({7, 7, 7, 7, 7}), "ranges::fill");

    values = {1, 1, 2, 2, 3, 3};
    const auto unique_tail = std::ranges::unique(values);
    values.erase(unique_tail.begin(), unique_tail.end());
    require(values == std::vector<int>({1, 2, 3}), "ranges::unique");

    values = {1, 2, 3, 4, 5};
    std::ranges::rotate(values, values.begin() + 2);
    require(values == std::vector<int>({3, 4, 5, 1, 2}), "ranges::rotate");

    std::mt19937 engine(123u);
    std::vector<int> sample;
    std::ranges::sample(values, std::back_inserter(sample), 3, engine);
    require(sample.size() == 3, "ranges::sample");

    log.require(true, "required ranges algorithms work");
}

void test_transform_if(Logger& log) {
    std::vector<int> values{1, 2, 3, 4, 5, 6};
    std::vector<int> result;

    transform_if(values, std::back_inserter(result), [](int value) {
        return value % 2 == 0;
    }, [](int value) {
        return value * value;
    });

    log.require(result == std::vector<int>({4, 16, 36}), "transform_if filters before transform");
}

void test_errors(Logger& log) {
    const std::vector<double> expected{1.0, 2.0, 3.0};
    const std::vector<double> actual{2.0, 2.0, 5.0};

    require(std::abs(mae(expected, actual) - 1.0) < 1e-9, "MAE");
    require(std::abs(mse(expected, actual) - 5.0 / 3.0) < 1e-9, "MSE");
    log.require(true, "MAE and MSE are computed by numeric algorithms");
}

void test_views(Logger& log) {
    std::vector<int> values{1, 2, 3, 4, 5, 6};
    const auto filtered = collect(values | std::views::filter([](int value) {
                                      return value % 2 == 0;
                                  }));
    require(filtered == std::vector<int>({2, 4, 6}), "views::filter");

    const auto dropped = collect(values | std::views::drop(3));
    require(dropped == std::vector<int>({4, 5, 6}), "views::drop");

    std::vector<std::vector<int>> nested{{1, 2}, {3}, {4, 5}};
    const auto joined = collect(nested | std::views::join);
    require(joined == std::vector<int>({1, 2, 3, 4, 5}), "views::join");

    std::vector<int> left{1, 2, 3};
    std::vector<int> right{4, 5, 6};
    std::vector<int> zipped_sums;
    for (auto [lhs, rhs] : std::views::zip(left, right)) {
        zipped_sums.push_back(lhs + rhs);
    }
    require(zipped_sums == std::vector<int>({5, 7, 9}), "views::zip");

    const auto strided = collect(std::views::iota(0, 10) | std::views::stride(3));
    require(strided == std::vector<int>({0, 3, 6, 9}), "views::stride");

    log.require(true, "required standard views work");
}

void test_fibonacci_view(Logger& log) {
    const auto values = collect(Fibonacci(8));
    log.require(values == std::vector<int>({0, 1, 1, 2, 3, 5, 8, 13}),
                "Fibonacci view generates the sequence");
}

void run_all() {
    Logger log;

    try {
        test_ranges_algorithms(log);
        test_transform_if(log);
        test_errors(log);
        test_views(log);
        test_fibonacci_view(log);
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
    return 0;
}
