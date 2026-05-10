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

// Problem: 11.04
// Description: Quadratic solver result extraction with std::visit and a Visitor.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using RootVariant = std::variant<double, std::pair<double, double>, std::monostate>;
using SolveResult = std::optional<RootVariant>;

SolveResult solve(double a, double b, double c) {
    constexpr double eps = 1e-12;

    if (std::abs(a) < eps) {
        if (std::abs(b) < eps) {
            return std::abs(c) < eps ? SolveResult{RootVariant{std::monostate{}}} : std::nullopt;
        }
        return RootVariant{-c / b};
    }

    const double discriminant = b * b - 4 * a * c;
    if (discriminant < -eps) {
        const double nan = std::nan("");
        return RootVariant{std::pair{nan, nan}};
    }
    if (std::abs(discriminant) < eps) {
        return RootVariant{-b / (2 * a)};
    }

    const double root = std::sqrt(discriminant);
    return RootVariant{std::pair{(-b - root) / (2 * a), (-b + root) / (2 * a)}};
}

struct Visitor {
    std::vector<double> operator()(double root) const {
        return {root};
    }

    std::vector<double> operator()(std::pair<double, double> roots) const {
        return {roots.first, roots.second};
    }

    std::vector<double> operator()(std::monostate) const {
        return {};
    }
};

std::vector<double> roots_from_result(const SolveResult& result) {
    if (!result) {
        return {};
    }

    return std::visit(Visitor{}, *result);
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

bool equal(double lhs, double rhs) {
    return std::abs(lhs - rhs) < 1e-9;
}

void test_one_root(Logger& log) {
    const auto roots = roots_from_result(solve(1, 2, 1));
    log.require(roots.size() == 1 && equal(roots.front(), -1.0), "visitor extracts one root");
}

void test_two_roots(Logger& log) {
    const auto roots = roots_from_result(solve(1, -3, 2));
    log.require(roots.size() == 2 && equal(roots[0], 1.0) && equal(roots[1], 2.0),
                "visitor extracts two roots");
}

void test_no_roots(Logger& log) {
    const auto roots = roots_from_result(solve(1, 0, 1));
    log.require(roots.size() == 2 && std::isnan(roots[0]) && std::isnan(roots[1]),
                "visitor keeps previous NaN pair behavior for complex roots");
}

void test_infinite_roots(Logger& log) {
    const SolveResult result = solve(0, 0, 0);
    log.require(result.has_value() && std::holds_alternative<std::monostate>(*result) &&
                    roots_from_result(result).empty(),
                "visitor handles monostate");
}

void run_all() {
    Logger log;

    test_one_root(log);
    test_two_roots(log);
    test_no_roots(log);
    test_infinite_roots(log);

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
