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
// Description: Hybrid sort on random-access iterators with a user comparator.

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

namespace hybrid_sort {

constexpr std::ptrdiff_t kInsertionThreshold = 16;

template <std::random_access_iterator Iterator, typename Compare>
void insertion_sort(Iterator first, Iterator last, Compare compare) {
    if (first == last) {
        return;
    }

    for (Iterator current = std::next(first); current != last; ++current) {
        Iterator position = current;

        while (position != first) {
            Iterator previous = std::prev(position);
            if (!compare(*position, *previous)) {
                break;
            }

            std::iter_swap(previous, position);
            position = previous;
        }
    }
}

template <std::random_access_iterator Iterator, typename Compare>
Iterator partition_hoare(Iterator first, Iterator last, Compare compare) {
    Iterator middle = first;
    std::advance(middle, std::distance(first, last) / 2);
    Iterator tail = std::prev(last);

    if (compare(*middle, *first)) {
        std::iter_swap(middle, first);
    }
    if (compare(*tail, *first)) {
        std::iter_swap(tail, first);
    }
    if (compare(*tail, *middle)) {
        std::iter_swap(tail, middle);
    }

    const auto pivot = *middle;
    Iterator left = first;
    Iterator right = tail;

    while (true) {
        while (compare(*left, pivot)) {
            ++left;
        }
        while (compare(pivot, *right)) {
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

template <std::random_access_iterator Iterator, typename Compare>
void sort_impl(Iterator first, Iterator last, Compare compare) {
    const std::ptrdiff_t size = std::distance(first, last);
    if (size <= 1) {
        return;
    }

    if (size <= kInsertionThreshold) {
        insertion_sort(first, last, compare);
        return;
    }

    const Iterator split = partition_hoare(first, last, compare);
    sort_impl(first, split, compare);
    sort_impl(split, last, compare);
}

template <std::random_access_iterator Iterator, typename Compare = std::less<>>
void sort(Iterator first, Iterator last, Compare compare = Compare{}) {
    sort_impl(first, last, compare);
}

}  // namespace hybrid_sort

namespace tests {

bool greater_int(int lhs, int rhs) {
    return lhs > rhs;
}

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

void test_free_function_comparator(Logger& log) {
    std::vector<int> data{5, 1, 4, 1, 3, 2};
    hybrid_sort::sort(data.begin(), data.end(), greater_int);
    log.require(std::ranges::is_sorted(data, greater_int), "free function comparator");
}

void test_standard_function_object(Logger& log) {
    std::deque<int> data{8, 3, 7, 2, 6, 1, 5, 4};
    hybrid_sort::sort(data.begin(), data.end(), std::less<>{});
    log.require(std::ranges::is_sorted(data, std::less<>{}), "std::less comparator");
}

void test_lambda_comparator(Logger& log) {
    std::vector<std::string> data{"bbb", "a", "cc", "dddd", "ee"};
    auto shorter = [](const std::string& lhs, const std::string& rhs) {
        if (lhs.size() != rhs.size()) {
            return lhs.size() < rhs.size();
        }
        return lhs < rhs;
    };

    hybrid_sort::sort(data.begin(), data.end(), shorter);
    log.require(std::ranges::is_sorted(data, shorter), "lambda comparator");
}

void test_default_comparator(Logger& log) {
    std::mt19937 engine(123456u);
    std::uniform_int_distribution<int> distribution(-100, 100);
    std::vector<int> data(1000);

    for (int& value : data) {
        value = distribution(engine);
    }

    hybrid_sort::sort(data.begin(), data.end());
    log.require(std::ranges::is_sorted(data), "default comparator");
}

void test_subrange(Logger& log) {
    std::vector<int> data{100, 5, 4, 3, 2, 1, 200};
    hybrid_sort::sort(std::next(data.begin()), std::prev(data.end()), std::greater<>{});

    const bool untouched_edges = data.front() == 100 && data.back() == 200;
    const bool sorted_middle =
        std::ranges::is_sorted(std::next(data.begin()), std::prev(data.end()), std::greater<>{});

    log.require(untouched_edges && sorted_middle, "half-open subrange with comparator");
}

void run_all() {
    Logger log;

    test_free_function_comparator(log);
    test_standard_function_object(log);
    test_lambda_comparator(log);
    test_default_comparator(log);
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
