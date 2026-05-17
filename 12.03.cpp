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

// Problem: 12.03
// Description: Find the longest palindromic substring using cached dynamic programming.

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::string longest_palindromic_substring(std::string_view text) {
    const std::size_t n = text.size();
    if (n == 0) {
        return {};
    }

    std::vector<bool> cache(n * n, false);
    std::size_t best_start = 0;
    std::size_t best_length = 1;

    for (std::size_t length = 1; length <= n; ++length) {
        for (std::size_t start = 0; start + length <= n; ++start) {
            std::size_t end = start + length - 1;
            bool is_palindrome = (text[start] == text[end]) &&
                (length <= 2 || cache[(start + 1) * n + (end - 1)]);
            cache[start * n + end] = is_palindrome;

            if (is_palindrome && length > best_length) {
                best_start = start;
                best_length = length;
            }
        }
    }

    return std::string(text.substr(best_start, best_length));
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

    void run_all() {
        Logger log;

        log.require(longest_palindromic_substring("a") == "a", "Single character string");
        log.require(longest_palindromic_substring("aa") == "aa", "Two identical characters");
        log.require(longest_palindromic_substring("ab") == "a", "Two distinct characters returns first char");
        log.require(longest_palindromic_substring("aba") == "aba", "Odd-length palindrome");
        log.require(longest_palindromic_substring("abba") == "abba", "Even-length palindrome");
        log.require(longest_palindromic_substring("babad") == "bab" || longest_palindromic_substring("babad") == "aba",
            "Longest palindrome in 'babad' is 'bab' or 'aba'");
        log.require(longest_palindromic_substring("cbbd") == "bb", "Longest palindrome in 'cbbd' is 'bb'");
        log.require(longest_palindromic_substring("forgeeksskeegfor") == "geeksskeeg",
            "Find longest palindrome in longer string");
        log.require(longest_palindromic_substring("") == "", "Empty string returns empty string");

        std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
        if (log.failed > 0) {
            std::exit(1);
        }
    }
}

int main() {
    tests::run_all();
    return 0;
}
