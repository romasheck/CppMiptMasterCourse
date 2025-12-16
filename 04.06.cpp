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

#include <array>
#include <cstddef>

namespace constants {
constexpr double kZero = 0.0;
constexpr double kOne = 1.0;
constexpr double kTwo = 2.0;
constexpr double kFour = 4.0;
}  // namespace constants

consteval double abs_consteval(double x) {
    return (x < constants::kZero) ? -x : x;
}

consteval double compute_e(double epsilon) {
    double e = constants::kZero;

    double term = constants::kOne;
    int n = 0;

    while (term > epsilon) {
        e += term;
        ++n;
        term /= static_cast<double>(n);
    }

    return e;
}

consteval double compute_pi(double epsilon) {
    double sum = constants::kZero;

    double term = constants::kOne;
    int n = 0;
    int sign = 1;

    while (abs_consteval(term) > epsilon) {
        sum += term;
        ++n;
        sign = -sign;
        const int denom = static_cast<int>(constants::kTwo) * n + 1;
        term = static_cast<double>(sign) / static_cast<double>(denom);
    }

    return constants::kFour * sum;
}

template <std::size_t N>
consteval std::array<double, N> compute_e_values(const std::array<double, N>& eps) {
    std::array<double, N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = compute_e(eps[i]);
    }
    return out;
}

template <std::size_t N>
consteval std::array<double, N> compute_pi_values(const std::array<double, N>& eps) {
    std::array<double, N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = compute_pi(eps[i]);
    }
    return out;
}

constexpr std::array<double, 3> kEpsilons{1e-1, 1e-2, 1e-3};

constexpr auto kEValues = compute_e_values(kEpsilons);
constexpr auto kPiValues = compute_pi_values(kEpsilons);

static_assert(kEValues[0] > 2.0 && kEValues[0] < 3.0);
static_assert(kEValues[1] > 2.7 && kEValues[1] < 2.8);
static_assert(kEValues[2] > 2.71 && kEValues[2] < 2.72);

static_assert(kPiValues[0] > 3.0 && kPiValues[0] < 4.0);
static_assert(kPiValues[1] > 3.1 && kPiValues[1] < 3.2);
static_assert(kPiValues[2] > 3.13 && kPiValues[2] < 3.15);

static_assert(abs_consteval(kEValues[1] - 2.718281828) < abs_consteval(kEValues[0] - 2.718281828));
static_assert(abs_consteval(kEValues[2] - 2.718281828) < abs_consteval(kEValues[1] - 2.718281828));

static_assert(abs_consteval(kPiValues[1] - 3.141592653) < abs_consteval(kPiValues[0] - 3.141592653));
static_assert(abs_consteval(kPiValues[2] - 3.141592653) < abs_consteval(kPiValues[1] - 3.141592653));

int main() {
    return 0;
}
