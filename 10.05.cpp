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

// Problem: 10.05
// Description: Longest common substring for integer sequences using dynamic programming.

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<int> find_longest_common_substring(const std::vector<int>& first,
                                               const std::vector<int>& second) {
    const std::size_t first_size = first.size();
    const std::size_t second_size = second.size();

    std::vector<std::vector<std::size_t>> dp(
        first_size + 1, std::vector<std::size_t>(second_size + 1, 0));

    std::size_t best_length = 0;
    std::size_t best_end = 0;

    for (std::size_t i = 1; i <= first_size; ++i) {
        for (std::size_t j = 1; j <= second_size; ++j) {
            if (first[i - 1] == second[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;

                if (dp[i][j] > best_length) {
                    best_length = dp[i][j];
                    best_end = i;
                }
            }
        }
    }

    if (best_length == 0) {
        return {};
    }

    return std::vector<int>(first.begin() + static_cast<std::ptrdiff_t>(best_end - best_length),
                            first.begin() + static_cast<std::ptrdiff_t>(best_end));
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

void test_exact_match(Logger& log) {
    const std::vector<int> result = find_longest_common_substring({1, 2, 3, 4}, {1, 2, 3, 4});
    require(result == std::vector<int>({1, 2, 3, 4}), "exact match should be returned");
    log.require(true, "exact match works");
}

void test_proper_substring(Logger& log) {
    const std::vector<int> result =
        find_longest_common_substring({7, 1, 2, 3, 8}, {0, 1, 2, 3, 9});

    require(result == std::vector<int>({1, 2, 3}), "expected contiguous common block");
    log.require(true, "proper substring is found");
}

void test_subsequence_is_not_enough(Logger& log) {
    const std::vector<int> result =
        find_longest_common_substring({1, 2, 3, 4, 5}, {1, 9, 3, 8, 5});

    require(result == std::vector<int>({1}), "algorithm should not jump over gaps");
    log.require(true, "substring differs from subsequence");
}

void test_no_common_substring(Logger& log) {
    const std::vector<int> result = find_longest_common_substring({1, 2, 3}, {4, 5, 6});
    require(result.empty(), "expected empty result");
    log.require(true, "empty result for disjoint sequences");
}

void test_duplicate_values(Logger& log) {
    const std::vector<int> result =
        find_longest_common_substring({1, 2, 2, 3, 4}, {9, 2, 2, 3, 8});

    require(result == std::vector<int>({2, 2, 3}), "expected longest duplicate-aware substring");
    log.require(true, "duplicate values are handled");
}

void run_all() {
    Logger log;

    try {
        test_exact_match(log);
        test_proper_substring(log);
        test_subsequence_is_not_enough(log);
        test_no_common_substring(log);
        test_duplicate_values(log);
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

    const std::vector<int> sample = find_longest_common_substring({5, 1, 2, 3, 4, 9},
                                                                  {7, 1, 2, 3, 8, 4});

    std::cout << "\nSample longest common substring: ";
    for (int value : sample) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    return 0;
}
