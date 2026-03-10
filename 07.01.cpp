#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 07.01 - quadratic solver with optional/variant
#include <iostream>
#include <optional>
#include <variant>
#include <utility>
#include <cmath>

using RootVariant = std::variant<double, std::pair<double,double>, std::monostate>;
using SolveResult = std::optional<RootVariant>;

SolveResult solve(double a, double b, double c)
{
    constexpr double EPS = 1e-12;
    if (std::abs(a) < EPS) {
        // not quadratic; handle degenerate cases
        if (std::abs(b) < EPS) {
            if (std::abs(c) < EPS) {
                // 0 == 0 -> infinite roots
                return RootVariant{std::monostate{}};
            } else {
                // constant nonzero -> no roots (optional empty)
                return std::nullopt;
            }
        }
        // linear equation bx + c = 0
        return RootVariant{-c / b};
    }
    double d = b*b - 4*a*c;
    if (d < -EPS) {
        double x1 = std::nan(""), x2 = std::nan("");
        return RootVariant{std::pair{x1,x2}};
    }
    if (std::abs(d) < EPS) return RootVariant{-b/(2*a)};
    double sqrt_d = std::sqrt(d);
    return RootVariant{std::pair{(-b - sqrt_d)/(2*a), (-b + sqrt_d)/(2*a)}};
}

namespace tests {
    struct Logger {
        int passed = 0, failed = 0;
        void require(bool cond, const char * msg) {
            if (cond) {
                ++passed;
                std::cout << "[OK]   " << msg << "\n";
            } else {
                ++failed;
                std::cout << "[FAIL] " << msg << "\n";
            }
        }
    };

    // compare doubles with tolerance
    constexpr double ROOT_EQ_EPS = 1e-9;
    bool eq(double x, double y) {
        return std::abs(x - y) < ROOT_EQ_EPS;
    }

    void run_all() {
        Logger log;

        {
            auto r = solve(0,1,2); // linear equation x + 2 = 0
            log.require(r.has_value(), "linear equation returns value");
            if (r) {
                bool ok = std::holds_alternative<double>(*r);
                log.require(ok, "linear returns single root");
                if (ok) log.require(eq(std::get<double>(*r), -2.0), "root -2");
            }
        }
        // constant nonzero has no roots
        log.require(!solve(0,0,1).has_value(), "constant !=0 -> no roots");

        auto r1 = solve(1,0,1);
        log.require(r1.has_value(), "d<0 returns value");
        if (r1) {
            auto &v = *r1;
            bool ok = std::holds_alternative<std::pair<double,double>>(v);
            log.require(ok, "pair variant");
            if (ok) {
                auto [x1,x2] = std::get<std::pair<double,double>>(v);
                log.require(std::isnan(x1) && std::isnan(x2), "NaN roots");
            }
        }

        auto r2 = solve(1,2,1);
        log.require(r2.has_value(), "d==0 returns value");
        if (r2) {
            bool ok = std::holds_alternative<double>(*r2);
            log.require(ok, "double variant");
            if (ok) {
                double x = std::get<double>(*r2);
                log.require(eq(x, -1.0), "root -1");
            }
        }

        auto r3 = solve(1,-3,2);
        log.require(r3.has_value(), "two roots");
        if (r3) {
            bool ok = std::holds_alternative<std::pair<double,double>>(*r3);
            log.require(ok, "pair variant");
            if (ok) {
                auto [x1,x2] = std::get<std::pair<double,double>>(*r3);
                log.require((eq(x1,1.0) && eq(x2,2.0)) ||
                            (eq(x1,2.0) && eq(x2,1.0)), "roots 1 and 2");
            }
        }

        auto inf = solve(0,0,0);
        log.require(inf.has_value(), "infinite roots case");
        log.require(std::holds_alternative<std::monostate>(*inf), "variant monostate");

        std::cout << "\n" << log.passed << " passed, " << log.failed << " failed\n";
        if (log.failed > 0) std::exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc>1 && std::string(argv[1]) == "--test") {
        tests::run_all();
        return 0;
    }
    double a,b,c;
    std::cin >> a >> b >> c;
    auto res = solve(a,b,c);
    if (!res) {
        std::cout << "not quadratic\n";
    } else {
        auto &v = *res;
        if (std::holds_alternative<double>(v)) {
            std::cout << "one root: " << std::get<double>(v) << "\n";
        } else if (std::holds_alternative<std::pair<double,double>>(v)) {
            auto [x1,x2] = std::get<std::pair<double,double>>(v);
            std::cout << "two roots: " << x1 << ", " << x2 << "\n";
        } else {
            std::cout << "infinite roots\n";
        }
    }
    return 0;
}
