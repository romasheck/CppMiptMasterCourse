#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out" -lgtest -pthread
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 07.04

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

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

    friend bool operator<(const Point& lhs, const Point& rhs) noexcept {
        if (lhs.x != rhs.x) {
            return lhs.x < rhs.x;
        }
        return lhs.y < rhs.y;
    }

    friend bool operator>(const Point& lhs, const Point& rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator==(const Point& lhs, const Point& rhs) noexcept {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

template <typename T>
void expect_matches_std_sort(std::vector<T> data) {
    std::vector<T> expected = data;
    std::sort(expected.begin(), expected.end());

    hybrid_sort::sort(data);

    EXPECT_EQ(data, expected);
    EXPECT_TRUE(std::is_sorted(data.begin(), data.end()));
}

TEST(HybridSort, EmptyVectorInt) {
    expect_matches_std_sort(std::vector<int>{});
}

TEST(HybridSort, SingleElementInt) {
    expect_matches_std_sort(std::vector<int>{42});
}

TEST(HybridSort, SmallIntegerCases) {
    expect_matches_std_sort(std::vector<int>{2, 1});
    expect_matches_std_sort(std::vector<int>{3, 1, 2});
    expect_matches_std_sort(std::vector<int>{5, 4, 3, 2, 1});
}

TEST(HybridSort, AlreadySortedInt) {
    std::vector<int> data(256);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    expect_matches_std_sort(std::move(data));
}

TEST(HybridSort, ReverseSortedInt) {
    const std::size_t size = 1000;
    std::vector<int> data(size);
    for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(size - i);
    }
    expect_matches_std_sort(std::move(data));
}

TEST(HybridSort, ManyEqualElements) {
    expect_matches_std_sort(std::vector<int>(1024, 7));
}

TEST(HybridSort, NearlySortedInt) {
    std::vector<int> data(512);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i);
    }
    std::swap(data[10], data[11]);
    std::swap(data[200], data[300]);
    expect_matches_std_sort(std::move(data));
}

TEST(HybridSort, RandomWithDuplicates) {
    std::mt19937 rng(123456u);
    std::uniform_int_distribution<int> dist(-50, 50);

    std::vector<int> data(2000);
    for (auto& value : data) {
        value = dist(rng);
    }

    expect_matches_std_sort(std::move(data));
}

TEST(HybridSort, DoubleValues) {
    expect_matches_std_sort(std::vector<double>{3.5, -1.25, 2.0, 2.0, 0.0, 100.125, -4.0});
}

TEST(HybridSort, Strings) {
    expect_matches_std_sort(std::vector<std::string>{
        "delta", "alpha", "charlie", "bravo", "alpha", "", "zeta", "echo",
    });
}

TEST(HybridSort, CustomPointType) {
    expect_matches_std_sort(std::vector<Point>{
        {2, 3}, {1, 9}, {2, 1}, {1, 2}, {2, 1}, {0, 0}, {1, 2},
    });
}

}  // namespace tests

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
