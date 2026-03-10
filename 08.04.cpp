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

// Problem: 08.04

#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace weasel {

constexpr char kFirstLetter = 'a';
constexpr char kLastLetter = 'z';
constexpr std::size_t kPopulationSize = 100;
constexpr double kMutationProbability = 0.05;
const std::string kTarget = "methinksitislikeaweasel";

template <typename Engine>
char random_letter(Engine& engine) {
    static std::uniform_int_distribution<int> distribution(kFirstLetter, kLastLetter);
    return static_cast<char>(distribution(engine));
}

template <typename Engine>
std::string random_string(std::size_t size, Engine& engine) {
    std::string result(size, kFirstLetter);
    for (char& ch : result) {
        ch = random_letter(engine);
    }
    return result;
}

int metric(const std::string& lhs, const std::string& rhs) {
    int result = 0;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            ++result;
        }
    }
    return result;
}

template <typename Engine>
std::string mutate(const std::string& source, Engine& engine) {
    std::uniform_real_distribution<double> probability(0.0, 1.0);
    std::string result = source;

    for (char& ch : result) {
        if (probability(engine) < kMutationProbability) {
            ch = random_letter(engine);
        }
    }

    return result;
}

template <typename Engine>
std::pair<std::string, int> next_generation(const std::string& parent, Engine& engine) {
    std::string best = parent;
    int best_metric = metric(parent, kTarget);

    for (std::size_t i = 0; i < kPopulationSize; ++i) {
        std::string candidate = mutate(parent, engine);
        const int candidate_metric = metric(candidate, kTarget);

        if (candidate_metric < best_metric) {
            best = std::move(candidate);
            best_metric = candidate_metric;
        }
    }

    return {best, best_metric};
}

bool is_lowercase_english(const std::string& value) {
    for (char ch : value) {
        if (ch < kFirstLetter || ch > kLastLetter) {
            return false;
        }
    }
    return true;
}

}  // namespace weasel

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

    {
        std::default_random_engine engine(12345u);
        const std::string value = weasel::random_string(weasel::kTarget.size(), engine);
        log.require(value.size() == weasel::kTarget.size(), "random string has target length");
        log.require(weasel::is_lowercase_english(value), "random string uses lowercase letters");
    }

    {
        log.require(weasel::metric("abc", "abc") == 0, "metric is zero for equal strings");
        log.require(weasel::metric("abc", "axz") == 2, "metric counts differing positions");
    }

    {
        std::default_random_engine engine(54321u);
        const std::string mutated = weasel::mutate(weasel::kTarget, engine);
        log.require(mutated.size() == weasel::kTarget.size(), "mutated string keeps size");
        log.require(weasel::is_lowercase_english(mutated), "mutated string uses lowercase letters");
    }

    {
        std::default_random_engine engine(42u);
        const std::string parent(weasel::kTarget.size(), 'a');
        const auto [best, best_metric] = weasel::next_generation(parent, engine);
        log.require(best.size() == weasel::kTarget.size(), "next generation keeps size");
        log.require(weasel::is_lowercase_english(best), "next generation keeps alphabet");
        log.require(best_metric <= weasel::metric(parent, weasel::kTarget),
                    "next generation never gets worse");
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();

    std::random_device device;
    std::default_random_engine engine(device());

    std::string current = weasel::random_string(weasel::kTarget.size(), engine);
    int current_metric = weasel::metric(current, weasel::kTarget);
    std::size_t iteration = 0;

    while (true) {
        std::cout << iteration << ' ' << current << ' ' << current_metric << '\n';

        if (current_metric == 0) {
            break;
        }

        const auto [next, next_metric] = weasel::next_generation(current, engine);
        current = next;
        current_metric = next_metric;
        ++iteration;
    }

    return 0;
}
