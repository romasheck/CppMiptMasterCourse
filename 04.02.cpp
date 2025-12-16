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

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>

namespace stats {

template <typename T>
double max_value(T value) {
    static_assert(std::is_same_v<std::remove_cvref_t<T>, double>, "double only");
    return value;
}

template <typename T, typename... Args>
double max_value(T first, Args... args) {
    static_assert(std::is_same_v<std::remove_cvref_t<T>, double>, "double only");
    static_assert((std::is_same_v<std::remove_cvref_t<Args>, double> && ...), "double only");
    const double max_rest = max_value(args...);
    return (first > max_rest) ? first : max_rest;
}

template <typename T>
double min_value(T value) {
    static_assert(std::is_same_v<std::remove_cvref_t<T>, double>, "double only");
    return value;
}

template <typename T, typename... Args>
double min_value(T first, Args... args) {
    static_assert(std::is_same_v<std::remove_cvref_t<T>, double>, "double only");
    static_assert((std::is_same_v<std::remove_cvref_t<Args>, double> && ...), "double only");
    const double min_rest = min_value(args...);
    return (first < min_rest) ? first : min_rest;
}

template <typename... Args>
double sum(Args... args) {
    static_assert(sizeof...(Args) > 0, "non-empty pack required");
    static_assert((std::is_same_v<std::remove_cvref_t<Args>, double> && ...), "double only");
    return (... + args);
}

template <typename... Args>
double average(Args... args) {
    static_assert(sizeof...(Args) > 0, "non-empty pack required");
    static_assert((std::is_same_v<std::remove_cvref_t<Args>, double> && ...), "double only");
    return (... + args) / static_cast<double>(sizeof...(args));
}

}  // namespace stats

namespace tests {

constexpr double kEps = 1e-12;

bool almost_equal(double a, double b) {
    return std::fabs(a - b) <= kEps;
}

struct Logger {
    std::size_t passed = 0;

    void pass(const std::string& name) {
        ++passed;
        std::cout << "PASS: " << name << '\n';
    }
};

void require(bool cond) {
    assert(cond);
}

void test_max(Logger& log) {
    require(almost_equal(stats::max_value(42.0), 42.0));
    log.pass("max_single");

    require(almost_equal(stats::max_value(1.0, 2.0, 3.0, 4.0, 5.0), 5.0));
    log.pass("max_increasing");

    require(almost_equal(stats::max_value(5.0, 4.0, 3.0, 2.0, 1.0), 5.0));
    log.pass("max_decreasing");

    require(almost_equal(stats::max_value(-1.0, -2.0, -3.0), -1.0));
    log.pass("max_all_negative");

    require(almost_equal(stats::max_value(3.14, 2.71, 1.41), 3.14));
    log.pass("max_mixed_constants");

    require(almost_equal(stats::max_value(2.0, 2.0, 2.0), 2.0));
    log.pass("max_all_equal");
}

void test_min(Logger& log) {
    require(almost_equal(stats::min_value(42.0), 42.0));
    log.pass("min_single");

    require(almost_equal(stats::min_value(1.0, 2.0, 3.0, 4.0, 5.0), 1.0));
    log.pass("min_increasing");

    require(almost_equal(stats::min_value(5.0, 4.0, 3.0, 2.0, 1.0), 1.0));
    log.pass("min_decreasing");

    require(almost_equal(stats::min_value(-1.0, -2.0, -3.0), -3.0));
    log.pass("min_all_negative");

    require(almost_equal(stats::min_value(3.14, 2.71, 1.41), 1.41));
    log.pass("min_mixed_constants");

    require(almost_equal(stats::min_value(2.0, 2.0, 2.0), 2.0));
    log.pass("min_all_equal");
}

void test_sum(Logger& log) {
    require(almost_equal(stats::sum(42.0), 42.0));
    log.pass("sum_single");

    require(almost_equal(stats::sum(1.0, 2.0, 3.0, 4.0, 5.0), 15.0));
    log.pass("sum_arithmetic_1_to_5");

    require(almost_equal(stats::sum(-1.0, 1.0, -2.0, 2.0), 0.0));
    log.pass("sum_canceling");

    require(almost_equal(stats::sum(0.1, 0.2, 0.3), 0.6));
    log.pass("sum_decimal");
}

void test_average(Logger& log) {
    require(almost_equal(stats::average(42.0), 42.0));
    log.pass("avg_single");

    require(almost_equal(stats::average(1.0, 2.0, 3.0, 4.0, 5.0), 3.0));
    log.pass("avg_1_to_5");

    require(almost_equal(stats::average(3.0, 4.0, 5.0), 4.0));
    log.pass("avg_3_4_5");

    require(almost_equal(stats::average(10.0, 20.0, 30.0), 20.0));
    log.pass("avg_tens");

    require(almost_equal(stats::average(-1.0, 0.0, 1.0), 0.0));
    log.pass("avg_symmetric");

    require(almost_equal(stats::average(0.1, 0.2, 0.3), 0.2));
    log.pass("avg_decimal");
}

void run_all() {
    Logger log;
    test_max(log);
    test_min(log);
    test_sum(log);
    test_average(log);
    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
