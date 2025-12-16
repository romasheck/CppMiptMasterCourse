#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out"
rm "${SCRIPT_NAME}.out"
exit
#endif

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace hybrid_sort {

constexpr std::size_t kInsertionThreshold = 16;

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
void quick_sort_impl(std::vector<T>& data, std::size_t left, std::size_t right) {
    if (right - left > kInsertionThreshold) {
        const std::size_t split = partition_hoare(data, left, right);
        quick_sort_impl(data, left, split);
        quick_sort_impl(data, split, right);
    } else {
        insertion_sort(data, left, right);
    }
}

template <typename T>
void sort(std::vector<T>& data) {
    if (data.empty()) {
        return;
    }
    quick_sort_impl(data, 0, data.size());
}

}  // namespace hybrid_sort

namespace tests {

struct Point {
    int x{};
    int y{};

    friend bool operator<(const Point& a, const Point& b) noexcept {
        if (a.x != b.x) {
            return a.x < b.x;
        }
        return a.y < b.y;
    }
    friend bool operator>(const Point& a, const Point& b) noexcept {
        return b < a;
    }
};

struct Logger {
    std::size_t passed = 0;

    void pass(const std::string& name) {
        ++passed;
        std::cout << "PASS: " << name << '\n';
    }
};

template <typename T>
void expect_sorted(const std::vector<T>& data, const std::string& test_name, Logger& log) {
    assert(std::ranges::is_sorted(data));
    log.pass(test_name);
}

void test_empty_vector(Logger& log) {
    std::vector<int> data;
    hybrid_sort::sort(data);
    expect_sorted(data, "empty_vector_int", log);
}

void test_single_element(Logger& log) {
    std::vector<int> data{42};
    hybrid_sort::sort(data);
    expect_sorted(data, "single_element_int", log);
}

void test_small_sizes(Logger& log) {
    {
        std::vector<int> data{2, 1};
        hybrid_sort::sort(data);
        expect_sorted(data, "two_elements_swap_int", log);
    }
    {
        std::vector<int> data{3, 1, 2};
        hybrid_sort::sort(data);
        expect_sorted(data, "three_elements_int", log);
    }
    {
        std::vector<int> data{5, 4, 3, 2, 1};
        hybrid_sort::sort(data);
        expect_sorted(data, "five_elements_desc_int", log);
    }
}

void test_already_sorted(Logger& log) {
    std::vector<int> data(256);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    hybrid_sort::sort(data);
    expect_sorted(data, "already_sorted_int_256", log);
}

void test_reverse_sorted(Logger& log) {
    const std::size_t size = 1000;
    std::vector<int> data(size);
    for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(size - i);
    }
    hybrid_sort::sort(data);
    expect_sorted(data, "reverse_sorted_int_1000", log);
}

void test_many_equal(Logger& log) {
    std::vector<int> data(1024, 7);
    hybrid_sort::sort(data);
    expect_sorted(data, "many_equal_int_1024", log);
}

void test_nearly_sorted(Logger& log) {
    std::vector<int> data(512);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    std::swap(data[10], data[11]);
    std::swap(data[200], data[300]);
    hybrid_sort::sort(data);
    expect_sorted(data, "nearly_sorted_int_512", log);
}

void test_random_with_duplicates(Logger& log) {
    std::mt19937 rng(123456u);
    std::uniform_int_distribution<int> dist(-50, 50);

    std::vector<int> data(2000);
    for (auto& v : data) {
        v = dist(rng);
    }

    hybrid_sort::sort(data);
    expect_sorted(data, "random_int_duplicates_2000_seeded", log);
}

void test_double(Logger& log) {
    std::vector<double> data{3.5, -1.25, 2.0, 2.0, 0.0, 100.125, -4.0};
    hybrid_sort::sort(data);
    expect_sorted(data, "double_mixed_small", log);
}

void test_string(Logger& log) {
    std::vector<std::string> data{
        "delta", "alpha", "charlie", "bravo", "alpha", "", "zeta", "echo",
    };
    hybrid_sort::sort(data);
    expect_sorted(data, "string_lexicographic_with_empty", log);
}

void test_custom_type(Logger& log) {
    std::vector<Point> data{
        {2, 3}, {1, 9}, {2, 1}, {1, 2}, {2, 1}, {0, 0}, {1, 2},
    };
    hybrid_sort::sort(data);
    expect_sorted(data, "custom_point_lexicographic", log);
}

void run_all() {
    Logger log;

    test_empty_vector(log);
    test_single_element(log);
    test_small_sizes(log);
    test_already_sorted(log);
    test_reverse_sorted(log);
    test_many_equal(log);
    test_nearly_sorted(log);
    test_random_with_duplicates(log);
    test_double(log);
    test_string(log);
    test_custom_type(log);

    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
