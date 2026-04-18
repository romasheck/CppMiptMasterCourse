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

// Problem: 09.04
// Description: Hybrid sort on random-access iterators and half-open ranges.

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace hybrid_sort {

constexpr std::ptrdiff_t kInsertionThreshold = 16;

template <std::random_access_iterator Iterator>
void insertion_sort(Iterator first, Iterator last) {
    if (first == last) {
        return;
    }

    for (Iterator current = std::next(first); current != last; ++current) {
        Iterator position = current;

        while (position != first) {
            Iterator previous = std::prev(position);
            if (!(*position < *previous)) {
                break;
            }

            std::iter_swap(previous, position);
            position = previous;
        }
    }
}

template <std::random_access_iterator Iterator>
Iterator partition_hoare(Iterator first, Iterator last) {
    Iterator middle = first;
    std::advance(middle, std::distance(first, last) / 2);
    Iterator tail = std::prev(last);

    if (*middle < *first) {
        std::iter_swap(middle, first);
    }
    if (*tail < *first) {
        std::iter_swap(tail, first);
    }
    if (*tail < *middle) {
        std::iter_swap(tail, middle);
    }

    const auto pivot = *middle;
    Iterator left = first;
    Iterator right = tail;

    while (true) {
        while (*left < pivot) {
            ++left;
        }
        while (pivot < *right) {
            --right;
        }

        if (left >= right) {
            return std::next(right);
        }

        std::iter_swap(left, right);
        ++left;
        --right;
    }
}

template <std::random_access_iterator Iterator>
void sort_impl(Iterator first, Iterator last) {
    const std::ptrdiff_t size = std::distance(first, last);
    if (size <= 1) {
        return;
    }

    if (size <= kInsertionThreshold) {
        insertion_sort(first, last);
        return;
    }

    const Iterator split = partition_hoare(first, last);
    sort_impl(first, split);
    sort_impl(split, last);
}

template <std::random_access_iterator Iterator>
void sort(Iterator first, Iterator last) {
    sort_impl(first, last);
}

}  // namespace hybrid_sort

namespace tests {

struct Point {
    int x{};
    int y{};

    friend bool operator<(const Point& lhs, const Point& rhs) noexcept {
        if (lhs.x != rhs.x) {
            return lhs.x < rhs.x;
        }
        return lhs.y < rhs.y;
    }
};

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

template <typename Container>
void expect_sorted(const Container& data, Logger& log, const char* message) {
    log.require(std::is_sorted(data.begin(), data.end()), message);
}

void test_empty_vector(Logger& log) {
    std::vector<int> data;
    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "empty vector");
}

void test_single_element(Logger& log) {
    std::vector<int> data{42};
    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "single element");
}

void test_small_vector(Logger& log) {
    std::vector<int> data{5, 4, 3, 2, 1};
    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "small vector");
}

void test_reverse_sorted(Logger& log) {
    std::vector<int> data(1000);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(data.size() - i);
    }

    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "reverse sorted vector");
}

void test_random_duplicates(Logger& log) {
    std::mt19937 rng(123456u);
    std::uniform_int_distribution<int> dist(-50, 50);

    std::vector<int> data(2000);
    for (int& value : data) {
        value = dist(rng);
    }

    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "random vector with duplicates");
}

void test_deque(Logger& log) {
    std::deque<int> data{9, 1, 7, 2, 8, 3, 6, 4, 5, 0};
    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "deque with random-access iterators");
}

void test_strings(Logger& log) {
    std::vector<std::string> data{"delta", "alpha", "charlie", "bravo", "alpha", ""};
    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "vector of strings");
}

void test_custom_type(Logger& log) {
    std::vector<Point> data{{2, 3}, {1, 9}, {2, 1}, {1, 2}, {0, 0}, {2, 1}};
    hybrid_sort::sort(data.begin(), data.end());
    expect_sorted(data, log, "custom comparable type");
}

void test_subrange(Logger& log) {
    std::vector<int> data{100, 5, 4, 3, 2, 1, 200};
    hybrid_sort::sort(std::next(data.begin()), std::prev(data.end()));

    const bool untouched_edges = data.front() == 100 && data.back() == 200;
    const bool sorted_middle =
        std::ranges::is_sorted(std::next(data.begin()), std::prev(data.end()));

    log.require(untouched_edges && sorted_middle, "half-open subrange sort");
}

void run_all() {
    Logger log;

    test_empty_vector(log);
    test_single_element(log);
    test_small_vector(log);
    test_reverse_sorted(log);
    test_random_duplicates(log);
    test_deque(log);
    test_strings(log);
    test_custom_type(log);
    test_subrange(log);

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

// Score is 9/10
